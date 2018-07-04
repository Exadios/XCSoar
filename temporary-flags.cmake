# These are compiler flags that are temporarily required to compile the code
# at this checkout. The most likely reason that is that the compiler or some
# other aspect of the build environment has changed since the time that this
# current checkout was written.
#
# These flags should be dispensed with ASAP.

set(temporary_cxx_flags "-Wno-error=unused-parameter -Wno-error=deprecated-declarations -Wno-error=shift-negative-value")

set(temporary_c_flags "-Wno-error=shift-negative-value")
