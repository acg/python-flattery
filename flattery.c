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

  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *
flatten(PyObject *ignore, PyObject *args)
{
  Py_INCREF(Py_None);
  return Py_None;
}


/* List of free functions defined in the module */

static PyMethodDef flattery_methods[] = {
  {"unflatten", unflatten, METH_VARARGS, "unflatten"},
  {"flatten", flatten, METH_VARARGS, "flatten"},
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

