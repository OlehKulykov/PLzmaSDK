
#include "../libplzma.hpp"

@interface PLzmaSDKOutStream() {
@protected
    plzma::SharedPtr<plzma::OutStream> _outStream;
}

@property (nonatomic, assign, readonly) const plzma::SharedPtr<plzma::OutStream> * outStreamSPtr;
@property (nonatomic, assign, readonly) BOOL isMulti;

- (instancetype) initWithOutStreamM:(plzma::SharedPtr<plzma::OutStream> *) outStream;
- (instancetype) initWithOutStreamMTh:(plzma::SharedPtr<plzma::OutStream> *) outStream;

@end
