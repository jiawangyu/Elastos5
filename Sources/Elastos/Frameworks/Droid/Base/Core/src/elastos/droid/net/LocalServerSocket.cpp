
#include "elastos/droid/net/LocalServerSocket.h"
#include "elastos/droid/net/CLocalSocketAddress.h"
#include "elastos/droid/net/CLocalSocket.h"

namespace Elastos {
namespace Droid {
namespace Net {

CAR_INTERFACE_IMPL(LocalServerSocket, Object, ILocalServerSocket)

const Int32 LocalServerSocket::LISTEN_BACKLOG = 50;

ECode LocalServerSocket::constructor(
    /* [in] */ const String& name)
{
    mImpl = new LocalSocketImpl();

    mImpl->Create(ILocalSocket::SOCKET_STREAM);

    CLocalSocketAddress::New(name, (ILocalSocketAddress**)&mLocalAddress);
    FAIL_RETURN(mImpl->Bind(mLocalAddress));
    return mImpl->Listen(LISTEN_BACKLOG);
}

ECode LocalServerSocket::constructor(
    /* [in] */ IFileDescriptor* fd)
{
    mImpl = new LocalSocketImpl();
    mImpl->constructor(fd);
    FAIL_RETURN(mImpl->Listen(LISTEN_BACKLOG));
    return mImpl->GetSockAddress((ILocalSocketAddress**)&mLocalAddress);
}

ECode LocalServerSocket::GetLocalSocketAddress(
    /* [out] */ ILocalSocketAddress** result)
{
    VALIDATE_NOT_NULL(result);

    *result = mLocalAddress;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode LocalServerSocket::Accept(
    /* [out] */ ILocalSocket** result)
{
    VALIDATE_NOT_NULL(result);

    LocalSocketImpl* acceptedImpl = new LocalSocketImpl();

    mImpl->Accept(acceptedImpl);

    return CLocalSocket::New(acceptedImpl, LocalSocket::SOCKET_UNKNOWN, result);
}

ECode LocalServerSocket::GetFileDescriptor(
    /* [out] */ IFileDescriptor** result)
{
    VALIDATE_NOT_NULL(result);

    return mImpl->GetFileDescriptor(result);
}

ECode LocalServerSocket::Close()
{
    return mImpl->Close();
}


} // namespace Net
} // namespace Droid
} // namespace Elastos