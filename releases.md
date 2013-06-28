Release Log
===========
v0.2.0 (6/28/2013)
- Migrates to a completely internal storage model using structs and pointers. This improves memory usage and speed immensely.
- Refactors the namespace storage so that we are storing as few strings as possible, then pointing to them from the event structures.
- Refactors the Statsrb::get() method to do less string comparison.
- Removes the @data property that formerly housed all of the time series data. This removes convenience, but was necessary.
- Adds a length method to see how large the data set is.
- Adds "big" data tests to stress the system a bit.
- Converts to rb_define_alloc_func() to be more appropriate.
- Converts Data_Wrap_Struct assignment to the whole object instead of a property.
- Adds Statsrb::clear() to clean out internal data.
- Adds Statsrb::load_test() to populate with test data.

v0.1.4 (6/10/2013)
- Adds a JS example to the readme file.
- Deprecates Statsrb::query() in favor of Statsrb::read().
- Adds Statsrb::get() method.
