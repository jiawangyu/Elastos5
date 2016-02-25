
#ifndef __ORG_APACHE_HTTP_IMPL_COOKIE_BASICCOMMENTHANDLER_H__
#define __ORG_APACHE_HTTP_IMPL_COOKIE_BASICCOMMENTHANDLER_H__

#include "org/apache/http/impl/cookie/AbstractCookieAttributeHandler.h"

using Org::Apache::Http::Cookie::ISetCookie;

namespace Org {
namespace Apache {
namespace Http {
namespace Impl {
namespace Cookie {

class BasicCommentHandler : public AbstractCookieAttributeHandler
{
public:
    CARAPI Parse(
        /* [in] */ ISetCookie* cookie,
        /* [in] */ const String& value);
};

} // namespace Cookie
} // namespace Impl
} // namespace Http
} // namespace Apache
} // namespace Org

#endif // __ORG_APACHE_HTTP_IMPL_COOKIE_BASICCOMMENTHANDLER_H__