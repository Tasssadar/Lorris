#! /bin/bash
LIBUSBY_REPO="git://github.com/avakar/libusby.git"
GOT_PYTHON=0
GOT_KATE=0
GOT_QSCI=0

function CHECK_EXECUTABLE {
    echo -n "Checking for $1..."
    exec_path=$(which $1)
    if [ -z $exec_path ] ; then
        echo ""
        echo ""
        echo "$1 not found!"
        exit 1
    else
        echo $exec_path
    fi
}

function REPORT_GIT_FAIL {
    echo ""
    echo "git submodule resulted in following output:"
    echo $1
    echo ""
    echo "That may not be actually an error, try to run this script once again"
    exit 1
}

function CHECK_SUBMODULE {
    IFS=$' '
    parts=($1)
    echo "  Synchronizing ${parts[1]}..."

    if [ ${#parts[@]} -eq 2 ] ; then
        res=$(git submodule --quiet init "${parts[1]}" 2>&1)
        if [ -n "$res" ] ; then
            REPORT_GIT_FAIL "$res"
        fi
    fi
    #res=$(git submodule update "${parts[1]}")
    res=$(git submodule --quiet update "${parts[1]}" 2>&1)
    if [ -n "$res" ] ; then
        REPORT_GIT_FAIL "$res"
    fi
    unset $IFS
}

echo "Checking required executables..."
CHECK_EXECUTABLE qmake
CHECK_EXECUTABLE pkg-config
CHECK_EXECUTABLE make
CHECK_EXECUTABLE cc
CHECK_EXECUTABLE c++

echo ""
echo "Checking Qt libraries..."

qtLibs=( 'QtCore' 'QtGui' 'QtNetwork' 'QtScript' 'QtXml' 'QtUiTools' )
for lib in ${qtLibs[@]} ; do
    echo -n "Checking for $lib..."
    libver=$(pkg-config --modversion $lib --silence-errors)
    if [ -z $libver ] ; then
        echo "$lib not found!"
        exit 1
    else
        if [ $(echo $libver | tr -d .) -lt 470 ] ; then
            echo "too old ($libver, minimum is 4.7.0)"
            exit 1
        else
            echo "ok ($libver)"
        fi
    fi
done

echo ""
echo -n "Checking if this is a git repository..."
IS_GIT=$( ( [ -d ".git" ] && echo 0) || echo 1)
if [ $IS_GIT -eq 0 ] ; then
    echo "yes"
else
    echo "no"
fi

if [ $IS_GIT -eq 0 ] ; then
    echo "Configuring submodules..."
    IFS=$'\n'
    submodules=$(git submodule status)
    unset $IFS
    for module in ${submodules[@]} ; do
        CHECK_SUBMODULE $module
    done
else
    echo -n "Checking libusby..."
    if [ -e "dep/libusby/libusby.pri" ] ; then
        echo "ok"
    else
        echo "not found"
        echo "Clone libusby repository ($LIBUSBY_REPO) into dep/libusby"
    fi
fi

# Check for SDL
echo ""
echo -n "Checking for lSDL..."
cat > /tmp/test.cpp << EOF
#include <SDL/SDL.h>
int main () { SDL_Init(SDL_INIT_VIDEO); return 0; }
EOF

c++ /tmp/test.cpp -o /tmp/test -lSDL&> /dev/null
if [ $? -eq 0 ] ; then
    echo "ok"
    rm /tmp/test.cpp
    rm /tmp/test
else
    echo "failed"
    echo "Install libSDL developement files!"
    rm /tmp/test.cpp
    exit 1
fi

# Check for python
echo ""
echo "Python support configuration"
REQ_PYTHON_VERSION=2.7
echo "Compatible python versions:"
echo "    [1]: 2.5"
echo "    [2]: 2.6"
echo "  > [3]: 2.7"
echo "    [4]: Do not build with python support"
echo -n "Enter your selection (1-4): "
read python_ver_user

case $python_ver_user in
    "1")
        REQ_PYTHON_VERSION=2.5
        ;;
    "2")
        REQ_PYTHON_VERSION=2.6
        ;;
    "3")
        REQ_PYTHON_VERSION=2.7
        ;;
    "4")
        REQ_PYTHON_VERSION=0
        echo "Not using python"
        ;;
esac

if [ "$REQ_PYTHON_VERSION" != "0" ] ; then
    echo -n "Checking for python $REQ_PYTHON_VERSION..."
    pkg-config --exists python-$REQ_PYTHON_VERSION
    if [ $? -eq 0 ] ; then 
        GOT_PYTHON=1
        echo "ok"
    else
        echo "not found!"
        GOT_PYTHON=0
    fi
fi

# Check for kate
echo ""
echo "Code editor configuration"
echo -n "Do you want to use Kate editor? [Y/n]: "
read use_kate_user
echo ""
if [ -z $use_kate_user ] || [ $use_kate_user == "y" ] || [ $use_kate_user == "Y" ] ; then
    echo -n "Checking for kate and kde developement files..."
    cat > /tmp/test.cpp << EOF
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/configinterface.h>
#include <kconfig.h>
int main() { KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor(); return 0; }
EOF
    c++ /tmp/test.cpp -o /tmp/test -I"$(qmake -query QT_INSTALL_HEADERS)" -lktexteditor -lkdecore &> /dev/null
    if [ $? -eq 0 ] ; then
        echo "ok"
        rm /tmp/test.cpp
        rm /tmp/test
        GOT_KATE=1
    else
        echo "failed"
        echo "Install kate and kdelibs devel files!"
        rm /tmp/test.cpp
        exit 1
    fi
else
    echo "Not using kate"
fi

echo -n "Do you want to use QScintilla2 editor? [y/N]: "
read use_qsci_user
echo ""
if [ "$use_qsci_user" == "y" ] || [ "$use_qsci_user" == "Y" ]; then
    echo -n "Checking for QScintilla dev files..."
    cat > /tmp/test.cpp << EOF
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerpython.h>
int main() { QsciScintilla *s = new QsciScintilla; delete s; return 0; }
EOF
    c++ /tmp/test.cpp -o /tmp/test -I"$(qmake -query QT_INSTALL_HEADERS)" -I"$(qmake -query QT_INSTALL_HEADERS)/QtCore" -I"$(qmake -query QT_INSTALL_HEADERS)/QtGui" -lqscintilla2
    if [ $? -eq 0 ] ; then
        echo "ok"
        rm /tmp/test.cpp
        rm /tmp/test
        GOT_QSCI=1
    else
        echo "failed"
        echo "Install qscintilla devel files!"
        rm /tmp/test.cpp
        exit 1
    fi
else
    echo "Not using QScintilla"
fi

# CONFIGURATION COMPLETE, GENERATE FILES
echo ""
echo -n "Generating config.pri.."
echo "" > config.pri

if [ $GOT_PYTHON -eq 1 ] ; then 
    echo "PYTHON_VERSION=$REQ_PYTHON_VERSION" >> config.pri
    echo "CONFIG += python" >> config.pri
fi

if [ $GOT_KATE -eq 1 ] ; then 
    echo "CONFIG += kate_editor" >> config.pri
fi

if [ $GOT_QSCI -eq 1 ] ; then 
    echo "CONFIG += qsci_editor" >> config.pri
fi

echo "done"
echo "Running qmake..."
qmake CONFIG+=release QMAKE_CXXFLAGS+="$CXXFLAGS" QMAKE_CFLAGS+="$CFLAGS" Lorris.pro

if [ $? -eq 0 ] ; then
    echo ""
    echo "Configuration succesfully completed. You can now run \"make\" to compile Lorris"
fi
exit 0

