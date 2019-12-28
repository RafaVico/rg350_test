#!/bin/sh

OPK_NAME=rg350test.opk

echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=RG350 test
Comment=Test your RG-350
Exec=rg350test.gcw
Terminal=false
Type=Application
StartupNotify=true
Icon=rg350test
Categories=applications;
EOF

# create opk
FLIST="media"
FLIST="${FLIST} rg350test.gcw"
FLIST="${FLIST} rg350test.png"
FLIST="${FLIST} default.gcw0.desktop"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop

