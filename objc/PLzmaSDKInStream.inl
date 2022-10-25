
#include "../libplzma.hpp"

@interface PLzmaSDKInStream() {
@private
    plzma::SharedPtr<plzma::InStream> _inStream;
}

@property (nonatomic, assign, readonly) const plzma::SharedPtr<plzma::InStream> * inStreamSPtr;

@end
