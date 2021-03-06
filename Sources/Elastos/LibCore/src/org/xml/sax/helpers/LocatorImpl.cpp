
#include "LocatorImpl.h"

namespace Org {
namespace Xml {
namespace Sax {
namespace Helpers {

CAR_INTERFACE_IMPL_2(LocatorImpl, Object, ILocator, ILocatorImpl)

LocatorImpl::LocatorImpl()
    : mLineNumber(0)
    , mColumnNumber(0)
{
}

ECode LocatorImpl::constructor()
{
    return NOERROR;
}

ECode LocatorImpl::constructor(
    /* [in] */ ILocator* locator)
{
    if (locator) {
        String id;
        locator->GetPublicId(&id);
        SetPublicId(id);

        locator->GetSystemId(&id);
        SetSystemId(id);

        Int32 number = 0;
        locator->GetLineNumber(&number);
        SetLineNumber(number);

        locator->GetColumnNumber(&number);
        SetColumnNumber(number);
    }

    return NOERROR;
}

ECode LocatorImpl::SetPublicId(
    /* [in] */ const String& publicId)
{
    mPublicId = publicId;
    return NOERROR;
}

ECode LocatorImpl::SetSystemId(
    /* [in] */ const String& systemId)
{
    mSystemId = systemId;
    return NOERROR;
}

ECode LocatorImpl::SetLineNumber(
    /* [in] */ Int32 lineNumber)
{
    mLineNumber = lineNumber;
    return NOERROR;
}

ECode LocatorImpl::SetColumnNumber(
    /* [in] */ Int32 columnNumber)
{
    mColumnNumber = columnNumber;
    return NOERROR;
}

ECode LocatorImpl::GetPublicId(
    /* [out] */ String* publicId)
{
    VALIDATE_NOT_NULL(publicId);

    *publicId = mPublicId;

    return NOERROR;
}

ECode LocatorImpl::GetSystemId(
    /* [out] */ String* systemId)
{
    VALIDATE_NOT_NULL(systemId);

    *systemId = mSystemId;

    return NOERROR;
}

ECode LocatorImpl::GetLineNumber(
    /* [out] */ Int32* lineNumber)
{
    VALIDATE_NOT_NULL(lineNumber);

    *lineNumber = mLineNumber;

    return NOERROR;
}

ECode LocatorImpl::GetColumnNumber(
    /* [out] */ Int32* columnNumber)
{
    VALIDATE_NOT_NULL(columnNumber);

    *columnNumber = mColumnNumber;

    return NOERROR;
}

} // namespace Helpers
} // namespace Sax
} // namespace Xml
} // namespace Org
