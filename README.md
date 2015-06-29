TINY LITTLE MOOLTIPASS LIB
==========================

Simple C lib to communicate with a [Mooltipass](http://themooltipass.com). Should be written entirely in C89 with only libusb as a dependency (also pkg-config for building)

Usage is simple and mostly asynchronous, although a few synchronous functions exist for ease of use where no user input on the device is required.

```
tlmpContext* ctx;
tlmpInitContext(&ctx);

tlmpSetConnectCallback(ctx, onConnect);

while(1)
{
    tlmpUpdateContext(ctx);
}

tlmpTerminateContext(&ctx);

void onConnect(tlmpContext* ctx, tlmpDevice* dev, int conn)
{
    if(conn)
    {
        tlmpRequestAuthentication(dev, "domain.com", onAuth);
    }
}

void onAuth(const char* login, const char* pass)
{
    // password only valid for the lifetime of this callback
    // login not guaranteed to be valid after this callback
}
```

The commands that the mooltipass understands are listed in src/include/tlmp_messageids.h. At this point the only ones used are:
`TLMP_MESSAGE_STATUS      (0xB9)`
`TLMP_MESSAGE_SETCONTEXT  (0xA3)`
`TLMP_MESSAGE_GETLOGIN    (0xA4)`
`TLMP_MESSAGE_GETPASSWORD (0xA5)`

TODO:
- Implement credential storage (`TLMP_MESSAGE_SETLOGIN` `TLMP_MESSAGE_SETPASSWORD`)
- Implement Mooltipass version checking (`TLMP_MESSAGE_VERSION`)
- Implement data storage (`TLMP_MESSAGE_SETDATACONTEXT` `TLMP_MESSAGE_ADDDATACONTEXT` `TLMP_MESSAGE_WRITEDATACONTEXT` `TLMP_MESSAGE_READDATACONTEXT`)
  - Create sample SSH wrapper using mooltipass storage for keys
- Implement password generator based on Mooltipass random number generator (`TLMP_MESSAGE_GETRANDOM`)
- Far, far, far more things