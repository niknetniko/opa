export PATH=/nix/store/maxmc03d48xwrnm5x7ys0hmsby4nnd91-devshell-dir/bin:$PATH

# LD_LIBRARY_PATH only needed if you are building without rpath
# export LD_LIBRARY_PATH=/nix/store/maxmc03d48xwrnm5x7ys0hmsby4nnd91-devshell-dir/lib:$LD_LIBRARY_PATH

export XDG_DATA_DIRS=/nix/store/maxmc03d48xwrnm5x7ys0hmsby4nnd91-devshell-dir/share:${XDG_DATA_DIRS:-/var/empty/local/share/:/var/empty/share/}
export XDG_CONFIG_DIRS=/nix/store/maxmc03d48xwrnm5x7ys0hmsby4nnd91-devshell-dir/etc/xdg:${XDG_CONFIG_DIRS:-/etc/xdg}

export QT_PLUGIN_PATH=/nix/store/maxmc03d48xwrnm5x7ys0hmsby4nnd91-devshell-dir/lib/plugins:$QT_PLUGIN_PATH
export QML2_IMPORT_PATH=/nix/store/maxmc03d48xwrnm5x7ys0hmsby4nnd91-devshell-dir/lib/qml:$QML2_IMPORT_PATH

export QT_QUICK_CONTROLS_STYLE_PATH=/nix/store/maxmc03d48xwrnm5x7ys0hmsby4nnd91-devshell-dir/lib/qml/QtQuick/Controls.2/:$QT_QUICK_CONTROLS_STYLE_PATH
