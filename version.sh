#!/bin/sh

set -e

VERSION_CODE=1000000
VERSION_NAME=0.10.0
VERSION_TARGET=$1

do_version_readme() {
    # README.md
    # cat README.md \
    # | sed "s/\(#compile 'tv.danmaku.ijk.media:ijkplayer-java:#\)[[:digit:]][[:digit:].]*\(#'#\)/\1:$VERSION_NAME\2/" \
    # > README.md.new

    cat README.md \
    | sed "s/\(compile \'tv.danmaku.ijk.media:ijkplayer-[[:alnum:]_]*:\)[[:digit:].]*[[:digit:]]/\1$VERSION_NAME/g" \
    | sed "s/\(git checkout -B latest k\)[[:digit:]][[:digit:].]*/\1$VERSION_NAME/g" \
    > README.md.new

    mv -f README.md.new README.md
}

do_version_gradle() {
    # android/ijkplayer/build.gradle
    cat android/ijkplayer/build.gradle \
    | sed "s/\(versionCode[[:space:]]*=[[:space:]]*\)[[:digit:]][[:digit:]]*/\1$VERSION_CODE/" \
    | sed "s/\(versionName[[:space:]]*=[[:space:]]*\)\"[[:digit:].]*[[:digit:]]\"/\1\"$VERSION_NAME\"/" \
    > android/ijkplayer/build.gradle.new

    mv -f android/ijkplayer/build.gradle.new android/ijkplayer/build.gradle



    # android/ijkplayer/gradle.properties
    cat android/ijkplayer/gradle.properties \
    | sed "s/\(VERSION_NAME=\)[[:digit:].]*[[:digit:]]/\1$VERSION_NAME/" \
    | sed "s/\(VERSION_CODE=\)[[:digit:]][[:digit:]]*/\1$VERSION_CODE/" \
    > android/ijkplayer/gradle.properties.new

    mv -f android/ijkplayer/gradle.properties.new android/ijkplayer/gradle.properties



    # android/ijkplayer/ijkplayer-exo/build.gradle
    cat android/ijkplayer/ijkplayer-exo/build.gradle \
    | sed "s/\(compile \'tv.danmaku.ijk.media:ijkplayer-[-_[:alpha:][:digit:]]*:\)[[:digit:].]*[[:digit:]]/\1$VERSION_NAME/g" \
    > android/ijkplayer/ijkplayer-exo/build.gradle.new

    mv -f android/ijkplayer/ijkplayer-exo/build.gradle.new android/ijkplayer/ijkplayer-exo/build.gradle



    # android/ijkplayer/ijkplayer-example/build.gradle
    cat android/ijkplayer/ijkplayer-example/build.gradle \
    | sed "s/\(ompile \'tv.danmaku.ijk.media:ijkplayer-[-_[:alpha:][:digit:]]*:\)[[:digit:].]*[[:digit:]]/\1$VERSION_NAME/g" \
    > android/ijkplayer/ijkplayer-example/build.gradle.new

    mv -f android/ijkplayer/ijkplayer-example/build.gradle.new android/ijkplayer/ijkplayer-example/build.gradle
}

do_version_xcode() {
    # examples/ios/IJKMediaDemo.xcodeproj/project.pbxproj

    cat examples/ios/IJKMediaDemo.xcodeproj/project.pbxproj \
    | sed "s/\(MARKETING_VERSION = \).*;/\1$VERSION_NAME;/g" \
    > examples/ios/IJKMediaDemo.xcodeproj/project.pbxproj.new

    mv -f examples/ios/IJKMediaDemo.xcodeproj/project.pbxproj.new examples/ios/IJKMediaDemo.xcodeproj/project.pbxproj

    cat examples/macos/IJKMediaMacDemo.xcodeproj/project.pbxproj \
    | sed "s/\(MARKETING_VERSION = \).*;/\1$VERSION_NAME;/g" \
    > examples/macos/IJKMediaMacDemo.xcodeproj/project.pbxproj.new

    mv -f examples/macos/IJKMediaMacDemo.xcodeproj/project.pbxproj.new examples/macos/IJKMediaMacDemo.xcodeproj/project.pbxproj

    sed -i "" "s/\(export IJK_VERSION=\)[[:digit:].]*[[:digit:]]/\1$VERSION_NAME/g" shell/version.sh
    
}

if [ "$VERSION_TARGET" = "readme" ]; then
    do_version_readme
elif [ "$VERSION_TARGET" = "gradle" ]; then
    do_version_gradle
elif [ "$VERSION_TARGET" = "show" ]; then
    echo $VERSION_NAME
elif [ "$VERSION_TARGET" = "xcode" ]; then
    do_version_xcode
else
    do_version_readme
    do_version_gradle
    do_version_xcode
fi

