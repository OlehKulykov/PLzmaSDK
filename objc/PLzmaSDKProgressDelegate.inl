
#include <memory>

#include "../libplzma.hpp"

@protocol PLzmaSDKProgressDelegatePrivate <NSObject>
@required

- (void) onCPath:(nullable const char *) cPath progress:(double) progress;

@end

class PLzmaSDKProgressDelegate final : public plzma::ProgressDelegate {
private:
    std::weak_ptr<PLzmaSDKProgressDelegate> _weakSelf;
    void * LIBPLZMA_NULLABLE _delegate = nullptr;
    
public:
    virtual void onProgress(void * LIBPLZMA_NULLABLE context, const plzma::String & path, const double progress);
    
    PLzmaSDKProgressDelegate(void * LIBPLZMA_NULLABLE delegate) noexcept : _delegate(delegate) { }
    virtual ~PLzmaSDKProgressDelegate() noexcept = default;
    
    static std::shared_ptr<PLzmaSDKProgressDelegate> create(void * LIBPLZMA_NULLABLE decoder);
};
