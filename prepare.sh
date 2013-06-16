#! /bin/bash
LIBYB_REPO="git://github.com/avakar/libyb.git"
LIBENJOY_REPO="git://github.com/Tasssadar/libenjoy.git"
GOT_PYTHON=0
GOT_KATE=0
GOT_QSCI=0
GOT_JOYSTICK=0
GOT_QTOPENGL=0

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

qmake_bin="qmake"
cpp_bin="c++"
# Handle qmake-qt4 only system
if [ -z "$(which $qmake_bin > /dev/null 2>&1)" ]; then qmake_bin="qmake-qt4"; fi
# Handle cpp only system
if [ -z "$(which $cpp_bin > /dev/null 2>&1)" ]; then cpp_bin="cpp"; fi;

CHECK_EXECUTABLE $qmake_bin
CHECK_EXECUTABLE pkg-config
CHECK_EXECUTABLE make
CHECK_EXECUTABLE cc
CHECK_EXECUTABLE $cpp_bin

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

# Special QtOpenGL check
echo -n "Checking for QtOpenGL..."
libver=$(pkg-config --modversion QtOpenGL --silence-errors)
if [ -z $libver ] ; then
     echo "not found, building without OpenGL"
     GOT_QTOPENGL=0
else
    if [ $(echo $libver | tr -d .) -lt 470 ] ; then
        echo "too old ($libver, minimum is 4.7.0)"
        GOT_QTOPENGL=0
    else
        echo "ok ($libver)"
        echo ""
        echo -n "Do you want to build parts that require OpenGL? [Y/n]: "
        read use_qtopengl_user
        echo ""
        if [ -z $use_qtopengl_user ] || [ "$use_qtopengl_user" == "y" ] || [ "$use_qtopengl_user" == "Y" ] ; then
            GOT_QTOPENGL=1
        fi
    fi
fi

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
    for module in $(git submodule status) ; do
        CHECK_SUBMODULE "$module"
    done
    unset $IFS
else
    echo -n "Checking libyb..."
    if [ -e "dep/libyb/libyb.pri" ] ; then
        echo "ok"
    else
        echo "not found"
        echo "Clone libyb repository ($LIBYB_REPO) into dep/libyb"
    fi

    echo -n "Checking libenjoy..."
    if [ -e "dep/libenjoy/libenjoy.pri" ] ; then
        echo "ok"
    else
        echo "not found"
        echo "Clone libenjoy repository ($LIBENJOY_REPO) into dep/libenjoy"
    fi
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
    $cpp_bin /tmp/test.cpp -o /tmp/test -I"$($qmake_bin -query QT_INSTALL_HEADERS)" -I"$($qmake_bin -query QT_INSTALL_HEADERS)/QtCore" -I"$($qmake_bin -query QT_INSTALL_HEADERS)/QtGui" -lktexteditor -lkdecore -lQtCore -lQtGui
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
echo -n "Use systemwide QScintilla2 library? [Y/n]: "
read use_qsci_system
echo ""
if [ "$use_qsci_user" == "y" ] || [ "$use_qsci_user" == "Y" ]; then
    if [ "$use_qsci_system" == "n" ] || [ "$use_qsci_system" == "N" ]; then
        echo -n "Checking for QScintilla dev files..."
        cat > /tmp/test.cpp << EOF
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerpython.h>
int main() { QsciScintilla *s = new QsciScintilla; delete s; return 0; }
EOF
        $cpp_bin /tmp/test.cpp -o /tmp/test -I"$($qmake_bin -query QT_INSTALL_HEADERS)" -I"$($qmake_bin -query QT_INSTALL_HEADERS)/QtCore" -I"$($qmake_bin -query QT_INSTALL_HEADERS)/QtGui" -lqscintilla2 -lQtGui -lQtCore
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
        GOT_QSCI=2
        echo "Using Lorris' QScintilla2 library"
    fi
else
    echo "Not using QScintilla"
fi

echo ""
echo -n "Do you want joystick support? [Y/n]: "
read use_joy_user
echo ""
if [ -z $use_joy_user ] || [ "$use_joy_user" == "y" ] || [ $use_joy_user == "Y" ] ; then
    GOT_JOYSTICK=1
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
elif [ $GOT_QSCI -eq 2 ] ; then
    echo "CONFIG += qsci_system" >> config.pri
fi

if [ $GOT_JOYSTICK -eq 1 ] ; then
    echo "CONFIG += joystick" >> config.pri
fi

if [ $GOT_QTOPENGL -eq 1 ] ; then
    echo "CONFIG += opengl" >> config.pri
fi

echo "done"
echo "Running qmake..."
$qmake_bin CONFIG+=release QMAKE_CXXFLAGS+="$CXXFLAGS" QMAKE_CFLAGS+="$CFLAGS" Lorris.pro

if [ $? -eq 0 ] ; then
    echo ""
    echo "Configuration succesfully completed. You can now run \"make\" to compile Lorris"
fi
exit 0

