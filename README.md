# ffmpeg-android-example

Example app that use ffmpeg library to play video.

## Requirements

1. Installed Android NDK,
2. Builded project https://github.com/HausnerR/ffmpeg-android-build.

## Howto

1. Make sure paths to SDK and NDK in local.properties are valid,
2. Make sure paths to pre-builded ffmpeg in Android.mk are valid,
3. Push test.mp4 to your phone internal storage root folder,
3. Build and run app.

## Important things

Remember that supported formats depends on ffmpeg compile params.
If you compile this example with not modified **ffmpeg-android-build** script, you only get support for playing local mp4 files.

This example by default use **h264_mediacodec** ffmpeg decoder. If it failed to initialize, you got error (no fallback written).

All errors are outputted to logcat. No popups or something. It's just example app.

## Inspiration

- http://enoent.fr/blog/2014/06/20/compile-ffmpeg-for-android/
- https://github.com/appunite/AndroidFFmpeg
