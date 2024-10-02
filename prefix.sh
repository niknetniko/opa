export PATH=/home/niko/Ontwikkeling/opa/.data/bin:$PATH

# LD_LIBRARY_PATH only needed if you are building without rpath
# export LD_LIBRARY_PATH=/home/niko/Ontwikkeling/opa/.data/lib:$LD_LIBRARY_PATH

export XDG_DATA_DIRS=/home/niko/Ontwikkeling/opa/.data/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}
export XDG_CONFIG_DIRS=/home/niko/Ontwikkeling/opa/.data/etc/xdg:${XDG_CONFIG_DIRS:-/etc/xdg}

export QT_PLUGIN_PATH=/home/niko/Ontwikkeling/opa/.data/lib/plugins:$QT_PLUGIN_PATH
export QML2_IMPORT_PATH=/home/niko/Ontwikkeling/opa/.data/lib/qml:$QML2_IMPORT_PATH

export QT_QUICK_CONTROLS_STYLE_PATH=/home/niko/Ontwikkeling/opa/.data/lib/qml/QtQuick/Controls.2/:$QT_QUICK_CONTROLS_STYLE_PATH

export MANPATH=/home/niko/Ontwikkeling/opa/.data/share/man:${MANPATH:-/usr/local/share/man:/usr/share/man}
