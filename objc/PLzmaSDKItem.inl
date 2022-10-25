
#include "../libplzma.hpp"

@interface PLzmaSDKItem() {
@private
    plzma::SharedPtr<plzma::Item> _item;
}

@property (nonatomic, assign, readonly) const plzma::SharedPtr<plzma::Item> * itemSPtr;

- (instancetype) initWithItemM:(plzma::SharedPtr<plzma::Item> *) item;

@end
