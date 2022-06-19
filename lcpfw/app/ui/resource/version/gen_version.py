#!python3
# -*- coding: utf-8 -*-
# 0xCCCCCCCC

import os
import sys

def main():
    if len(sys.argv) < 3:
        assert len(sys.argv) >= 3, 'args not enough, need chromium version.py and *.rc files output dir.'
        return

    chromium_ver_py = sys.argv[1]
    out_dir = sys.argv[2]
    curdir = os.path.split(os.path.realpath(__file__))[0]
    #print('curdir: %s' % curdir)
    #print('chromium py: %s' % chromium_ver_py)
    #print('*.rc outdir: %s' % out_dir)
    # livehime
    #print('Generating livehime version')
    ret = os.system('python %s '
              ' -i %s/win.rc.in'
              ' -f %s/VERSION'
              ' -f %s/BRANDING'
              ' -f %s/LASTCHANGE'
              ' -f %s/livehime.ver'
              ' -o %s/livehime.rc'
              %(chromium_ver_py,curdir,curdir,curdir,curdir,curdir,out_dir))
    if ret != 0:
        assert False, 'Generate livehime version failed.'
        return
    # main_lib
    #print('Generating livehime_main_lib version')
    ret = os.system('python %s '
              ' -i %s/win.rc.in'
              ' -f %s/VERSION'
              ' -f %s/BRANDING'
              ' -f %s/LASTCHANGE'
              ' -f %s/livehime_main_lib.ver'
              ' -o %s/livehime_main_lib.rc'
              %(chromium_ver_py,curdir,curdir,curdir,curdir,curdir,out_dir))
    if ret != 0:
        assert False, 'Generate livehime_main_lib version failed.'
        return
    # secret_lib
    #print('Generating livehime_secret_lib version')
    ret = os.system('python %s '
              ' -i %s/win.rc.in'
              ' -f %s/VERSION'
              ' -f %s/BRANDING'
              ' -f %s/LASTCHANGE'
              ' -f %s/livehime_secret_lib.ver'
              ' -o %s/livehime_secret_lib.rc'
              %(chromium_ver_py,curdir,curdir,curdir,curdir,curdir,out_dir))
    if ret != 0:
        assert False, 'Generate livehime_secret_lib version failed.'
        return

if __name__ == '__main__':
    main()
