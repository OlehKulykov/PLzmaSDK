
#include <memory>

#include "../libplzma.hpp"

#import "PLzmaSDKProgressDelegate.inl"

@interface PLzmaSDKDecoder() <PLzmaSDKProgressDelegatePrivate> {
@private
    plzma::SharedPtr<plzma::Decoder> _decoder;
    std::shared_ptr<PLzmaSDKProgressDelegate> _progressDelegate;
}

@property (nonatomic, weak) id<PLzmaSDKDecoderDelegate> delegate;

@end
