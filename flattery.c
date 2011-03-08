#include "Python.h"
#include <ctype.h> // for isdigit()
#include <stdlib.h> // for atol()


static PyObject *
unflatten(PyObject *ignore, PyObject *args)
{
  PyObject *src = NULL;
  PyObject *dst = NULL;
  PyObject *nonelist = NULL;
  PyObject *slot = NULL;
  PyObject *slotvalue = NULL;
  PyObject *part = NULL;

  if (!PyArg_ParseTuple(args, "O!:unflatten", &PyDict_Type, &src))
    return NULL;

  if (!(dst = PyDict_New()))
    goto error;

  /* Create a [None] list. Used for extending lists to higher indices. */

  if (!(nonelist = PyList_New(1)))
    goto error;
  if (PyList_SetItem(nonelist, 0, Py_None) < 0)
    goto error;

  /* Iterate through key value pairs in the src dict,
     building the nested data structure in dst as we go. */

  PyObject *k, *v;
  Py_ssize_t pos = 0;

  while (PyDict_Next(src, &pos, &k, &v))
  {
    const char *key = PyString_AsString(k);
    const char *p;

    p = key;
    slot = dst;
    Py_INCREF(slot);

    do
    {
      /* Extract current part of the key path. */

      const char *start = p;
      while (*p && *p != '.') p++;
      part = PyString_FromStringAndSize(start, p-start);

      /* Advance to next part of key path, unless at the end. */

      if (*p == '.')
        p++;

      /* What value should we insert under this slot?
         - if this is the last path part, insert the value from src.
         - if the next path part is numeric, insert an empty list.
         - otherwise, insert an empty hash.
       */

      if (!*p) {
        slotvalue = v;
        Py_INCREF(slotvalue);
      }
      else if (isdigit(*p))
        slotvalue = PyList_New(0);
      else
        slotvalue = PyDict_New();

      if (!slotvalue)
        goto error;

      /* Assign to the current slot. */

      if (isdigit(*start))
      {
        /* If the current path part is numeric, index into a list.
           Extend the list with [None,None,...] if necessary. */

        if (!PyList_Check(slot))
          goto error;

        // FIXME thorough error checking here

        Py_ssize_t len = PyList_Size(slot);
        Py_ssize_t index = atol(PyString_AsString(part));

        if (index >= len)
        {
          PyObject *tail = PySequence_Repeat(nonelist, index-len+1);
          PyObject *extended = PySequence_InPlaceConcat(slot, tail);
          Py_XDECREF(tail);
          Py_XDECREF(extended);
        }

        /* Don't clobber an existing entry.
           PyXXX_SetItem(..., slotvalue) steals a reference to slotvalue. */

        PyObject *extant = NULL;

        if ((extant = PyList_GetItem(slot, index)) == Py_None)
          PyList_SetItem(slot, index, slotvalue);
        else {
          Py_DECREF(slotvalue);
          slotvalue = extant;
        }
      }
      else
      {
        /* If the current path part is non-numeric, index into a dict. */

        if (!PyDict_Check(slot))
          goto error;

        /* Don't clobber an existing entry.
           PyXXX_SetItem(..., slotvalue) steals a reference to slotvalue. */

        PyObject *extant = NULL;

        if (!(extant = PyDict_GetItem(slot, part)))
          PyDict_SetItem(slot, part, slotvalue);
        else {
          Py_DECREF(slotvalue);
          slotvalue = extant;
        }
      }

      /* Descend further into the dst data structure. */

      Py_INCREF(slotvalue);
      Py_DECREF(slot);
      slot = slotvalue;
      slotvalue = NULL;

      Py_DECREF(part);
      part = NULL;
    }
    while (*p);

    Py_DECREF(slot);
    slot = NULL;
  }

  return dst;

error:

  Py_XDECREF(dst);
  Py_XDECREF(nonelist);
  Py_XDECREF(slot);
  Py_XDECREF(slotvalue);
  Py_XDECREF(part);

  return NULL;
}


static PyObject *
flatten_internal(PyObject *src)
{
  PyObject *flat = NULL;
  PyObject *dst = NULL;

  if (PyList_Check(src))
  {
    if (!(flat = PyDict_New()))
      goto error;

    /* Iterate through elements in the list src, recursively flattening.
       Skip any entries which are None -- use a sparse encoding. */

    Py_ssize_t i;
    Py_ssize_t len = PyList_Size(src);

    for (i=0; i<len; i++)
    {
      PyObject *elem = PyList_GetItem(src,i);
      if (elem == Py_None && i<len-1) continue;
      Py_INCREF(elem);
      PyObject *o = flatten_internal(elem);
      Py_DECREF(elem);
      PyObject *k = PyString_FromFormat("%zd",i);
      PyDict_SetItem(flat, k, o);
    }
  }
  else if (PyDict_Check(src))
  {
    if (!(flat = PyDict_New()))
      goto error;

    /* Iterate through pairs in the dict src, recursively flattening. */

    PyObject *k, *v;
    Py_ssize_t pos = 0;

    while (PyDict_Next(src, &pos, &k, &v))
    {
      Py_INCREF(v);
      PyObject *o = flatten_internal(v);
      Py_DECREF(v);
      PyDict_SetItem(flat, k, o);
      Py_INCREF(k);
    }
  }
  else
  {
    /* The Python object is a scalar or something we don't know how
       to flatten, return it as-is. */

    return src;
  }

  /* Roll up recursively flattened dictionaries. */

  if (!(dst = PyDict_New()))
    goto error;

  PyObject *k1, *v1;
  Py_ssize_t pos1 = 0;

  while (PyDict_Next(flat, &pos1, &k1, &v1))
  {
    if (PyDict_Check(v1))
    {
      PyObject *k2, *v2;
      Py_ssize_t pos2 = 0;

      while (PyDict_Next(v1, &pos2, &k2, &v2))
      {
        const char *k1c = PyString_AsString(k1);
        const char *k2c = PyString_AsString(k2);
        PyObject *k = PyString_FromFormat("%s.%s",k1c,k2c);
        PyDict_SetItem(dst, k, v2);
        Py_INCREF(v2);
      }
    }
    else
    {
      PyDict_SetItem(dst, k1, v1);
      Py_INCREF(k1);
      Py_INCREF(v1);
    }
  }

  Py_DECREF(flat);

  return dst;

error:

  Py_XDECREF(dst);
  Py_XDECREF(flat);

  return NULL;
}


static PyObject *
flatten(PyObject *ignore, PyObject *args)
{
  PyObject *src = NULL;

  if (!PyArg_ParseTuple(args, "O!:flatten", &PyDict_Type, &src))
    return NULL;

  return flatten_internal(src);
}


/* List of free functions defined in the module */

static PyMethodDef flattery_methods[] = {
  {"unflatten", unflatten, METH_VARARGS, "unflatten(dict) -> dict"},
  {"flatten", flatten, METH_VARARGS, "flatten(dict) -> dict"},
  {NULL, NULL}    /* sentinel */
};

PyDoc_STRVAR(module_doc, "Flattery: fast flattening and unflattening of nested data structures.");

/* Initialization function for the module (*must* be called initcext) */

PyMODINIT_FUNC
initcext(void)
{
  /* Create the module and add the functions */

  PyObject* mod = Py_InitModule3("flattery.cext", flattery_methods, module_doc);
  if (mod == NULL)
    return;
}

