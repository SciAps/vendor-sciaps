#!/bin/bash

adb push $ANDROID_PRODUCT_OUT/system/bin/screencast /system/bin/
adb shell screencast
