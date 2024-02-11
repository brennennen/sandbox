

BUILD_SCRIPTS_DIR=$(dirname "$BASH_SOURCE")
SCRIPTS_DIR="$BUILD_SCRIPTS_DIR/.."
PROJECT_DIR="$SCRIPTS_DIR/.."
BUILD_DIR="$PROJECT_DIR/.build"
CALLED_FROM_DIR=$pwd

TOOLCHAIN_FILE="$PROJECT_DIR/toolchains/lynxos_toolchain.cmake"

# TODO: get lynxos
# TODO: source SETUP.bash
# TODO: call lwsmgr to grab a license
# TODO: add try catch logic to release the license if an error occurs

mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake .. -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE
make
#ctest --verbose
cd $CALLED_FROM_DIR
