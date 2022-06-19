# 资源封包 ###################################
实际实现方式是使用python脚本把xml文件、python文件里面描述的文件、字符串等资源按一定格式封装到一个文件里

generated_resources.grd
    用于指定要生成什么语言包，以及语言包对应的.xtb翻译文件，还有生成的相应.h头文件名称
    
resource_ids
    一个python文件，里面指定了资源构建时的文件源目录，资源包描述文件.grd中各个类型的资源的起始id值，
    资源类型包括：
        messages：字符串资源
        structures：图片资源，jpg、png
        includes：其他类型的资源
        
theme_resources.grd
    指定生成多少倍数的图片资源包，以及生成的资源项ID记录的.h文件名称
    

用法：
    python chromium\tools\grit\grit.py -i generated_resources.grd build -f resource_ids -o out
    python chromium\tools\grit\grit.py -i theme_resources.grd build -f resource_ids -o out
真实用例：
    /tools/grit/grit.py -i ../../../app/resources/ui_strings.grd build -o gen/app/resources --depdir . --depfile gen/app/resources/ui_strings_grd_grit.d --write-only-new=1 --depend-on-stamp -E root_gen_dir=gen -E root_src_dir=../../../ -D SHARED_INTERMEDIATE_DIR=gen -D scale_factors=2x -D _chromium -E CHROMIUM_BUILD=chromium -D toolkit_views -D use_aura -f F:/ProjectCode/mysite/lcpfw/app/resources/resource_ids -f gen/tools/gritsettings/default_resource_ids -p ../../../tools/gritsettings/startup_resources_win.txt --assert-file-list obj/app/resources/ui_strings_grd_expected_outputs.txt

grit查看帮助：
    python grit.py help
    
grd标签含义
    1、.grd文件中的判断语句<if expr="is_macosx">，这个expr应该是通过和python中的环境变量进行匹配？
       是通过执行脚本时的命令参数标识“-D xxx”来指定的



# 版本信息文件生成 ###################################
实际实现方式是使用python脚本，输入：
    1、-i，指定版本信息的模板文件（文件里的具体版本信息用变量名占位）；
    2、-f，以key=value的方式记录的实际版本信息文件（里面的key与1中的模板文件中的占位变量同名，python脚本用value替换模板中的变量）
    3、-o，输出文件
    
BRANDING
    里面记录的是不需要根据每次构建而动态变更的信息，比如公司名、版权信息等等

VERSION
    里面记录的是每次构建都有可能动态修改的信息，比如版本号

win.rc.version
    windows下.rc文件的版本信息段的模板文件

用法：
    python chromium\build\util\version.py -i "F:\ProjectCode\cef_study\resources\version/win.rc.version" -f "F:\ProjectCode\cef_study\resources\version/VERSION" -f "F:\ProjectCode\cef_study\resources\version/BRANDING" -f "F:\ProjectCode\cef_study\resources\version/LASTCHANGE" -f "F:\ProjectCode\cef_study\resources\version/livehime.ver" -o "F:\ProjectCode\cef_study\resources\version/test.rc"

TODO：
    mac下程序的版本信息还不知道怎么搞，是不是plist？