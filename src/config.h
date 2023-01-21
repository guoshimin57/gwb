/* *************************************************************************
 *     config.h：gwb的配置文件。
 *     版權 (C) 2023 gsm <406643764@qq.com>
 *     本程序為自由軟件：你可以依據自由軟件基金會所發布的第三版或更高版本的
 * GNU通用公共許可證重新發布、修改本程序。
 *     雖然基于使用目的而發布本程序，但不負任何擔保責任，亦不包含適銷性或特
 * 定目標之適用性的暗示性擔保。詳見GNU通用公共許可證。
 *     你應該已經收到一份附隨此程序的GNU通用公共許可證副本。否則，請參閱
 * <http://www.gnu.org/licenses/>。
 * ************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#define PREFIX "/usr" // gwb的安裝路徑前綴，應與Makefile的prefix保持一致
#define STD_UI_FILE PREFIX"/share/gwb/gwb.ui" // 安裝於系統中的UI文件全名
#define HOME_PAGE "www.bing.com" // 主頁
#define SHOW_HOME_PAGE 1 // 是否顯示主頁，0表示不顯示，非0整數表示顯示
#define DEFAULT_URI (SHOW_HOME_PAGE ? HOME_PAGE : "about:blank") // 默認URI
#define FOCUS_NEW_TAB_PAGE 1 // 是否聚焦新標籤頁，0表示不聚焦，非0整數表示聚焦
#define SEARCH_ENTRY_WIDTH_CHARS 20 // 以英文字符數量計算的搜索框寬度
#define TAB_LABEL_WIDTH_CHARS 15 // 以英文字符數量計算的標籤頁的標籤寬度
#define FAVOR_LABEL_WIDTH_CHARS 6 // 以英文字符數量計算的書籤的標籤寬度
#define MAIN_WIN_WIDTH 1024 // gwb主窗口的寬度
#define MAIN_WIN_HEIGHT 768 // gwb主窗口的高度

#endif
