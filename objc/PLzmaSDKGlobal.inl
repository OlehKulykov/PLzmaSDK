
#include <exception>

#include "../libplzma.hpp"

FOUNDATION_EXPORT NSException * PLzmaSDKPLExceptionToNS(const plzma::Exception & exception);
FOUNDATION_EXPORT NSException * PLzmaSDKSTDExceptionToNS(const std::exception & exception);
FOUNDATION_EXPORT NSException * PLzmaSDKExceptionToNS(void);
FOUNDATION_EXPORT NSException * PLzmaSDKExceptionCreate(const PLzmaSDKErrorCode errorCode, NSString * what, NSString * reason, const char * file, const int line);

#define PLZMASDKOBJC_TRY \
try { \


#define PLZMASDKOBJC_CATCH_RETHROW \
} catch (const plzma::Exception & exception) { \
    [PLzmaSDKPLExceptionToNS(exception) raise]; \
} catch (const std::exception & exception) { \
    [PLzmaSDKSTDExceptionToNS(exception) raise]; \
} catch (...) { \
    [PLzmaSDKExceptionToNS() raise];\
} \


