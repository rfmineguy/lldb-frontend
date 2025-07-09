#import <Foundation/Foundation.h>
#include "ThemeListener.hpp"

void RegisterThemeChangeObserver(void (*callback)(void)) {
    [[NSDistributedNotificationCenter defaultCenter]
        addObserverForName:@"AppleInterfaceThemeChangedNotification"
                    object:nil
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification *note) {
                    callback();
                }];
}
