dataclient
==========

The `dataclient` component of the `mearec` application provides a 
library implementing clients of the data servers from which multi-
electrode array data may be streamed. The `DataClient` class is
the base class, defining the public API used to interact with
servers. Inheriting from them are two subclasses which specialize
the class for interacting with MCS arrays (`McsClient`) and
HiDens arrays (`HidensClient`).

