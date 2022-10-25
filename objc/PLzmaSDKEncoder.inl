
#include <memory>

#include "../libplzma.hpp"

#import "PLzmaSDKProgressDelegate.inl"

@interface PLzmaSDKEncoder() <PLzmaSDKProgressDelegatePrivate> {
@private
    plzma::SharedPtr<plzma::Encoder> _encoder;
    std::shared_ptr<PLzmaSDKProgressDelegate> _progressDelegate;
}

@property (nonatomic, weak) id<PLzmaSDKEncoderDelegate> delegate;

@end
