# flattery: fast flattening and unflattening of nested Python data #

This library exposes a fast C implementation for flattening and unflattening hierarchical Python data structures. A unit test suite is included.

## Examples ##

### Synopsis ###

    from flattery import flatten, unflatten
    data = { "x.y.0", "zero", "x.y.1": "one", "x.z" : 42 }
    print unflatten(data)
    >>>
    { 'x' : { 'y' : [ 'zero', 'one' ], 'z': 42 }
    >>>
    assert( data == flatten(unflatten(data)) )

### Processing hierarchical records ###

    from flattery import unflatten
    cols = [ "time", "request.method", "request.uri", "response.status", "response.size" ]
    for line in sys.stdin:
      fields = line.rstrip('\r\n').split(len(fields)-1)
      values = dict([(cols[i],fields[i]) for i in xrange(len(cols))])
      record = unflatten(values)
      # do something with the record...
      print record
    >>>
    { 'time': '12/12/2012 12:00:00',
      'request': { 'method': 'GET', 'uri': '/stuff/' },
      'response': { 'status': '200', 'size': '40391' } }
    ...

### Web form processing: grouping data ###

Or suppose you have a web form for collecting several distinct blobs of data at once, like a payment form:

    <form method="post" action="">
      <div>
        First name: <input type="text" name="contact.firstname" value="" />
        Last name:  <input type="text" name="contact.lastname" value="" />
        Email:      <input type="text" name="contact.email" value="" />
      </div>
      <div>
        Credit Card Type: <select name="payment.cctype"> <option> ... </option> </select>
        Number:           <input type="text" name="payment.ccnumber" value="" />
        CCV:              <input type="text" name="payment.ccv" value="" />
        Expiration Month: <select name="payment.ccmonth"> <option> ... </option> </select>
        Expiration Year:  <select name="payment.ccyear"> <option> ... </option> </select>
      </div>
      <div>
        <input type="submit" name="submit" />
      </div>
    </form>

In the form processing code, you can expand the key value form data pairs into a nested object\*:

    from flattery import unflatten
    params = formdata()   # however you get a dictionary of form data...
    data = unflatten(params)
    print data
    >>>
    { 'contact':
        { 'lastname':'Doe',
          'firstname': 'John',
          'email':'jdoe@example.com' },
      'payment':
        { 'cctype': 'amex',
          'ccnumber': '4111111111111111',
          'ccv': '4321',
          'ccmonth': '12' ,
          'ccyear' : '2020' } }

(Be careful with multiply-valued form data.)

### Web form processing: tabular data ###

Another web example, where a user is editing tabular data\*:

    <form method="post" action="">

      <div>
        <ul class="table">
          <ul class="row">
            <li><input type="checkbox" name="rows.0.delete" /></li>
            <li><input type="text" name="rows.0.name" value="" /></li>
            <li><input type="text" name="rows.0.email" value="" /></li>
          </ul>
          <ul class="row">
            <li><input type="checkbox" name="rows.1.delete" /></li>
            <li><input type="text" name="rows.1.name" value="" /></li>
            <li><input type="text" name="rows.1.email" value="" /></li>
          </ul>
          ...
          <ul class="row">
            <li><input type="checkbox" name="rows.99.delete" /></li>
            <li><input type="text" name="rows.99.name" value="" /></li>
            <li><input type="text" name="rows.99.email" value="" /></li>
          </ul>
        </ul>
      </div>
      <div>
        <in.put type="submit" name="submit" />
      </div>
    </form>

(You should validate that indices are below some reasonable limit to avoid a memory DoS.)

In the form processing code:

    from flattery import unflatten
    params = formdata()   # however you get a dictionary of form data...
    data = unflatten(params)
    print data
    >>>
    { 'rows':
      [
        'delete': '1',
        'name': 'John Doe',
        'email': 'johndoe@example.com',
      ],
      [
        'delete': '0',
        'name': 'Suzy Q',
        'email': 'suzyq@example.com',
      ],
      ...
      [
        'delete': '0',
        'name': 'Charlie Chaplin',
        'email': 'charliechaplin@example.com',
      ],
    }

## Installation ##

Ubuntu / Debian users:

    fakeroot ./debian/rules binary
    dpkg -i ../python-flattery*.deb

If there's no "real" packaging for your system yet:

    ./setup.py build_ext --inplace
    ./test.py
    ./setup.py build
    ./setup.py install

