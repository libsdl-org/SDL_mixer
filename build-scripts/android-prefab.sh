#!/bin/bash

set -e

if ! [ "x${ANDROID_NDK_HOME}" != "x" -a -d "${ANDROID_NDK_HOME}" ]; then
    echo "ANDROID_NDK_HOME environment variable is not set"
    exit 1
fi

if ! [ "x${ANDROID_HOME}" != "x" -a -d "${ANDROID_HOME}" ]; then
    echo "ANDROID_HOME environment variable is not set"
    exit 1
fi

ANDROID_PLATFORM="${ANDROID_PLATFORM:-16}"

if [ "x${android_platform}" = "x" ]; then
    ANDROID_API="$(ls "${ANDROID_HOME}/platforms" | grep -E "^android-[0-9]+$" | sed 's/android-//' | sort -n -r | head -1)"
    if [ "x${ANDROID_API}" = "x" ]; then
        echo "No Android platform found in $ANDROID_HOME/platforms"
        exit 1
    fi
else
    if ! [ -d "${ANDROID_HOME}/platforms/android-${ANDROID_API}" ]; then
        echo "Android api version ${ANDROID_API} is not available (${ANDROID_HOME}/platforms/android-${ANDROID_API} does not exist)" >2
        exit 1
    fi
fi

android_platformdir="${ANDROID_HOME}/platforms/android-${ANDROID_API}"

echo "Building with ANDROID_PLATFORM=${ANDROID_PLATFORM}"
echo "android_platformdir=${android_platformdir}"

scriptdir=$(cd -P -- "$(dirname -- "$0")" && printf '%s\n' "$(pwd -P)")
sdlmixer_root=$(cd -P -- "$(dirname -- "$0")/.." && printf '%s\n' "$(pwd -P)")

build_root="${sdlmixer_root}/build-android-prefab"

android_abis="armeabi-v7a arm64-v8a x86 x86_64"
android_api=19
android_ndk=21
android_stl="c++_shared"

sdlmixer_major=$(sed -ne 's/^#define SDL_MIXER_MAJOR_VERSION  *//p' "${sdlmixer_root}/include/SDL3_mixer/SDL_mixer.h")
sdlmixer_minor=$(sed -ne 's/^#define SDL_MIXER_MINOR_VERSION  *//p' "${sdlmixer_root}/include/SDL3_mixer/SDL_mixer.h")
sdlmixer_patch=$(sed -ne 's/^#define SDL_MIXER_PATCHLEVEL  *//p' "${sdlmixer_root}/include/SDL3_mixer/SDL_mixer.h")
sdlmixer_version="${sdlmixer_major}.${sdlmixer_minor}.${sdlmixer_patch}"
echo "Building Android prefab package for SDL_mixer version $sdlmixer_version"

if test ! -d "${sdl_build_root}"; then
    echo "sdl_build_root is not defined or is not a directory."
    echo "Set this environment folder to the root of an android SDL${sdlmixer_major} prefab build"
    echo "This usually is SDL/build-android-prefab"
    exit 1
fi

prefabhome="${build_root}/prefab-${sdlmixer_version}"
rm -rf "$prefabhome"
mkdir -p "${prefabhome}"

build_cmake_projects() {
    for android_abi in $android_abis; do

        rm -rf "${build_root}/build_${android_abi}/prefix"

        for build_shared_libs in ON OFF; do
            echo "Configuring CMake project for $android_abi (shared=${build_shared_libs})"
            cmake -S "${sdlmixer_root}" -B "${build_root}/build_${android_abi}/shared_${build_shared_libs}" \
                -DSDL3MIXER_DEPS_SHARED=ON \
                -DSDL3MIXER_VENDORED=ON \
                -DSDL3MIXER_FLAC=ON \
                -DWITH_ASM=OFF \
                -DSDL3MIXER_FLAC_LIBFLAC=ON \
                -DSDL3MIXER_MOD=ON \
                -DSDL3MIXER_MOD_MODPLUG=OFF \
                -DSDL3MIXER_MOD_XMP=ON \
                -DSDL3MIXER_MP3=ON \
                -DSDL3MIXER_MP3_MPG123=ON \
                -DSDL3MIXER_MIDI=ON \
                -DSDL3MIXER_MIDI_TIMIDITY=ON \
                -DSDL3MIXER_OPUS=ON \
                -DSDL3MIXER_VORBIS=STB \
                -DSDL3MIXER_WAVPACK=ON \
                -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
                -DSDL${sdlmixer_major}_DIR="${sdl_build_root}/build_${android_abi}/prefix/lib/cmake/SDL${sdlmixer_major}" \
                -DANDROID_PLATFORM=${ANDROID_PLATFORM} \
                -DANDROID_ABI=${android_abi} \
                -DBUILD_SHARED_LIBS=${build_shared_libs} \
                -DCMAKE_INSTALL_PREFIX="${build_root}/build_${android_abi}/prefix" \
                -DCMAKE_INSTALL_INCLUDEDIR=include \
                -DCMAKE_INSTALL_LIBDIR=lib \
                -DCMAKE_BUILD_TYPE=Release \
                -DSDL${sdlmixer_major}MIXER_SAMPLES=OFF \
                -GNinja

            echo "Building CMake project for $android_abi (shared=${build_shared_libs})"
            cmake --build "${build_root}/build_${android_abi}/shared_${build_shared_libs}"

            echo "Installing CMake project for $android_abi (shared=${build_shared_libs})"
            cmake --install "${build_root}/build_${android_abi}/shared_${build_shared_libs}"
        done
    done
}

pom_filename="SDL${sdlmixer_major}_mixer-${sdlmixer_version}.pom"
pom_filepath="${prefabhome}/${pom_filename}"
create_pom_xml() {
    echo "Creating ${pom_filename}"
    cat >"${pom_filepath}" <<EOF
<project>
  <modelVersion>4.0.0</modelVersion>
  <groupId>org.libsdl.android</groupId>
  <artifactId>SDL${sdlmixer_major}_mixer</artifactId>
  <version>${sdlmixer_version}</version>
  <packaging>aar</packaging>
  <name>SDL${sdlmixer_major}_mixer</name>
  <description>The AAR for SDL${sdlmixer_major}_mixer</description>
  <url>https://libsdl.org/</url>
  <licenses>
    <license>
      <name>zlib License</name>
      <url>https://github.com/libsdl-org/SDL_mixer/blob/main/LICENSE.txt</url>
      <distribution>repo</distribution>
    </license>
  </licenses>
  <developers>
    <developer>
      <name>Sam Lantinga</name>
      <email>slouken@libsdl.org</email>
      <organization>SDL</organization>
      <organizationUrl>https://www.libsdl.org</organizationUrl>
    </developer>
  </developers>
  <scm>
    <connection>scm:git:https://github.com/libsdl-org/SDL_mixer</connection>
    <developerConnection>scm:git:ssh://github.com:libsdl-org/SDL_mixer.git</developerConnection>
    <url>https://github.com/libsdl-org/SDL_mixer</url>
  </scm>
  <distributionManagement>
    <snapshotRepository>
      <id>ossrh</id>
      <url>https://s01.oss.sonatype.org/content/repositories/snapshots</url>
    </snapshotRepository>
    <repository>
      <id>ossrh</id>
      <url>https://s01.oss.sonatype.org/service/local/staging/deploy/maven2/</url>
    </repository>
  </distributionManagement>
  <plugin>
    <groupId>com.simpligility.maven.plugins</groupId>
    <artifactId>android-maven-plugin</artifactId>
    <version>4.6.0</version>
    <extensions>true</extensions>
    <configuration>
      <sign>
        <debug>false</debug>
      </sign>
    </configuration>
  </plugin>
</project>
EOF
}

create_aar_androidmanifest() {
    echo "Creating AndroidManifest.xml"
    cat >"${aar_root}/AndroidManifest.xml" <<EOF
<manifest
    xmlns:android="http://schemas.android.com/apk/res/android"
    package="org.libsdl.android" android:versionCode="1"
    android:versionName="1.0">
	<uses-sdk android:minSdkVersion="16"
              android:targetSdkVersion="29"/>
</manifest>
EOF
}

echo "Creating AAR root directory"
aar_root="${prefabhome}/SDL${sdlmixer_major}_mixer-${sdlmixer_version}"
mkdir -p "${aar_root}"

aar_metainfdir_path=${aar_root}/META-INF
mkdir -p "${aar_metainfdir_path}"
cp "${sdlmixer_root}/LICENSE.txt" "${aar_metainfdir_path}"

prefabworkdir="${aar_root}/prefab"
mkdir -p "${prefabworkdir}"

cat >"${prefabworkdir}/prefab.json" <<EOF
{
  "schema_version": 2,
  "name": "SDL${sdlmixer_major}_mixer",
  "version": "${sdlmixer_version}",
  "dependencies": ["SDL${sdlmixer_major}"]
}
EOF

modulesworkdir="${prefabworkdir}/modules"
mkdir -p "${modulesworkdir}"

create_shared_sdl_mixer_module() {
    echo "Creating SDL${sdlmixer_major}_mixer prefab module"
    for android_abi in $android_abis; do
        sdl_moduleworkdir="${modulesworkdir}/SDL${sdlmixer_major}_mixer"
        mkdir -p "${sdl_moduleworkdir}"

        abi_build_prefix="${build_root}/build_${android_abi}/prefix"

        cat >"${sdl_moduleworkdir}/module.json" <<EOF
{
  "export_libraries": ["//SDL${sdlmixer_major}:SDL${sdlmixer_major}"],
  "library_name": "libSDL${sdlmixer_major}_mixer"
}
EOF
        mkdir -p "${sdl_moduleworkdir}/include/SDL${sdlmixer_major}"
        cp -r "${abi_build_prefix}/include/SDL${sdlmixer_major}/"* "${sdl_moduleworkdir}/include/SDL${sdlmixer_major}"

        abi_sdllibdir="${sdl_moduleworkdir}/libs/android.${android_abi}"
        mkdir -p "${abi_sdllibdir}"
        cat >"${abi_sdllibdir}/abi.json" <<EOF
{
  "abi": "${android_abi}",
  "api": ${android_api},
  "ndk": ${android_ndk},
  "stl": "${android_stl}",
  "static": false
}
EOF
        cp "${abi_build_prefix}/lib/libSDL${sdlmixer_major}_mixer.so" "${abi_sdllibdir}"
    done
}

create_static_sdl_mixer_module() {
    echo "Creating SDL${sdlmixer_major}_mixer-static prefab module"
    for android_abi in $android_abis; do
        sdl_moduleworkdir="${modulesworkdir}/SDL${sdlmixer_major}_mixer-static"
        mkdir -p "${sdl_moduleworkdir}"

        abi_build_prefix="${build_root}/build_${android_abi}/prefix"

        cat >"${sdl_moduleworkdir}/module.json" <<EOF
{
  "export_libraries": ["//SDL${sdlmixer_major}:SDL${sdlmixer_major}-static"],
  "library_name": "libSDL${sdlmixer_major}_mixer"
}
EOF
        mkdir -p "${sdl_moduleworkdir}/include/SDL${sdlmixer_major}"
        cp -r "${abi_build_prefix}/include/SDL${sdlmixer_major}/"* "${sdl_moduleworkdir}/include/SDL${sdlmixer_major}"

        abi_sdllibdir="${sdl_moduleworkdir}/libs/android.${android_abi}"
        mkdir -p "${abi_sdllibdir}"
        cat >"${abi_sdllibdir}/abi.json" <<EOF
{
  "abi": "${android_abi}",
  "api": ${android_api},
  "ndk": ${android_ndk},
  "stl": "${android_stl}",
  "static": true
}
EOF
        cp "${abi_build_prefix}/lib/libSDL${sdlmixer_major}_mixer.a" "${abi_sdllibdir}"
    done
}

create_shared_module() {
    modulename=$1
    libraryname=$2
    export_libraries=$3
    echo "Creating $modulename prefab module"

    export_libraries_json="[]"
    if test "x$export_libraries" != "x"; then
        export_libraries_json="["
        for export_library in $export_libraries; do
            if test "x$export_libraries_json" != "x["; then
                export_libraries_json="$export_libraries_json, "
            fi
            export_libraries_json="$export_libraries_json\"$export_library\""
        done
        export_libraries_json="$export_libraries_json]"
    fi

    for android_abi in $android_abis; do
        sdl_moduleworkdir="${modulesworkdir}/$modulename"
        mkdir -p "${sdl_moduleworkdir}"

        abi_build_prefix="${build_root}/build_${android_abi}/prefix"

        cat >"${sdl_moduleworkdir}/module.json" <<EOF
{
  "export_libraries": $export_libraries_json,
  "library_name": "$libraryname"
}
EOF

        abi_sdllibdir="${sdl_moduleworkdir}/libs/android.${android_abi}"
        mkdir -p "${abi_sdllibdir}"
        cat >"${abi_sdllibdir}/abi.json" <<EOF
{
  "abi": "${android_abi}",
  "api": ${android_api},
  "ndk": ${android_ndk},
  "stl": "${android_stl}"
}
EOF
        cp "${abi_build_prefix}/lib/$libraryname.so" "${abi_sdllibdir}"
    done
}

build_cmake_projects

create_pom_xml

create_aar_androidmanifest

create_shared_sdl_mixer_module

create_static_sdl_mixer_module

create_shared_module external_libogg libogg ""
cp "${sdlmixer_root}/external/ogg/COPYING" "${aar_metainfdir_path}/LICENSE.libogg.txt"

create_shared_module external_libflac libFLAC ":external_libogg"
for ext in FDL GPL LGPL Xiph; do
  cp "${sdlmixer_root}/external/flac/COPYING.$ext" "${aar_metainfdir_path}/LICENSE.libflac.$ext.txt"
done

create_shared_module external_libmpg123 libmpg123 ""
cp "${sdlmixer_root}/external/mpg123/COPYING" "${aar_metainfdir_path}/LICENSE.libmpg123.txt"

create_shared_module external_libopus libopus ":external_libogg"
cp "${sdlmixer_root}/external/opus/COPYING" "${aar_metainfdir_path}/LICENSE.libopus.txt"

create_shared_module external_libopusfile libopusfile ":external_libopus"
cp "${sdlmixer_root}/external/opusfile/COPYING" "${aar_metainfdir_path}/LICENSE.libopusfile.txt"

create_shared_module external_libxmp libxmp ""
tail -n15 "${sdlmixer_root}/external/libxmp/README" >"${aar_metainfdir_path}/LICENSE.libxmp.txt"

create_shared_module external_libwavpack libwavpack ""
tail -n15 "${sdlmixer_root}/external/wavpack/COPYING" >"${aar_metainfdir_path}/LICENSE.wavpack.txt"

pushd "${aar_root}"
    aar_filename="SDL${sdlmixer_major}_mixer-${sdlmixer_version}.aar"
    zip -r "${aar_filename}" AndroidManifest.xml prefab META-INF
    zip -Tv "${aar_filename}" 2>/dev/null ;
    mv "${aar_filename}" "${prefabhome}"
popd

maven_filename="SDL${sdlmixer_major}_mixer-${sdlmixer_version}.zip"

pushd "${prefabhome}"
    zip_filename="SDL${sdlmixer_major}_mixer-${sdlmixer_version}.zip"
    zip "${maven_filename}" "${aar_filename}" "${pom_filename}" 2>/dev/null;
    zip -Tv "${zip_filename}" 2>/dev/null;
popd

echo "Prefab zip is ready at ${prefabhome}/${aar_filename}"
echo "Maven archive is ready at ${prefabhome}/${zip_filename}"
