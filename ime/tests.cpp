// tests.cpp --- mzimeja のテスト。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include <shlobj.h>
#include <strsafe.h>
#include <clocale>
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

// テストエントリーを処理する。
void DoEntry(const std::wstring& pre, LPCWSTR post = NULL, BOOL show_graphviz = FALSE)
{
    MzConvResult result;
    TheIME.ConvertMultiClause(pre, result, show_graphviz);
    auto got = result.get_str();
    printf("%ls\n\n", got.c_str());
    if (post)
    {
        if (got != post)
        {
            printf("%ls\n\n", result.get_str(true).c_str());
            ASSERT(0);
        }
    }
    else
    {
        printf("%ls\n\n", result.get_str(true).c_str());
    }
}

// 動詞のテスト。
void DoDoushi(void)
{
    DoEntry(L"よせる。よせない。よせるとき。よせれば。よせろよ。よせてよ。",
            L"寄せる|。|寄せない|。|寄せる|とき|。|寄せれ|ば|。|寄せろよ|。|寄せて|よ|。");

    DoEntry(L"たべる。たべない。たべます。たべた。たべるとき。たべれば。たべろ。たべよう。",
            L"食べる|。|食べない|。|食べ|ます|。|食べ|た|。|食べる|とき|。|食べれ|ば|。|食べろ|。|食べよう|。");

    DoEntry(L"かきます。かいて。かかない。かく。かいた。かける。かこう。",
            L"書き|ます|。|書いて|。|書か|ない|。|書く|。|書いた|。|書ける|。|書こう|。");
    DoEntry(L"かけ。かくな。かけば。かかれる。かかせる。かかせられる。",
            L"書け|。|書くな|。|書け|ば|。|書か|れる|。|書か|せる|。|書かせ|られる|。");

    DoEntry(L"なかないで。ないた。なける。なこう。");
    DoEntry(L"よぶ。よんで。よばない。よべる。", L"呼ぶ|。|呼んで|。|呼ば|ない|。|呼べる|。");
    DoEntry(L"もつ。もって。もたない。もった。もてる。もとう。",
            L"持つ|。|持って|。|持た|ない|。|持った|。|持てる|。|持とう|。");

    DoEntry(L"みます。みて。みない。みる。みた。みられる。みよう。",
            L"見|ます|。|見て|。|見ない|。|見る|。|見た|。|見られる|。|見よう|。");
    DoEntry(L"みろ。みるな。みれば。みられる。みさせる。みさせられる。",
            L"見ろ|。|見るな|。|見れ|ば|。|見られる|。|見させる|。|見させ|られる|。");

    DoEntry(L"やってみます。やってみて。やってみない。やってみる。やってみた。やってみられる。やってみよう。",
            L"やってみ|ます|。|やってみて|。|やってみない|。|やってみる|。|やってみ|た|。|やってみ|られる|。|やってみよう|。");
    DoEntry(L"やってみろ。やってみるな。やってみれば。やってみられる。やってみさせる。やってみさせられる。",
            L"やってみろ|。|やってみるな|。|やってみれ|ば|。|やってみ|られる|。|やってみ|させる|。|やってみ|させ|られる|。");

    DoEntry(L"きます。きて。こない。くる。きた。こられる。こよう。");
    DoEntry(L"こい。くるな。くれば。こられる。こさせる。こさせられる。");

    DoEntry(L"やってきます。やってきて。やってこない。やってくる。やってきた。やってこられる。やってこよう。",
            L"やって来|ます|。|やって来|て|。|やって来|ない|。|やって来る|。|やって来|た|。|やって来|られる|。|やって来|よう|。");
    DoEntry(L"やってこい。やってくるな。やってくれば。やってこられる。やってこさせる。やってこさせられる。",
            L"やって来い|。|やって来るな|。|やって来れ|ば|。|やって来|られる|。|やって来|させる|。|やって来させ|られる|。");

    DoEntry(L"かいてんします。かいてんして。かいてんしない。");
    DoEntry(L"かいてんする。かいてんした。かいてんできる。かいてんしよう。");
    DoEntry(L"かいてんしろ。かいてんするな。かいてんすれば。");
    DoEntry(L"かいてんされる。かいてんさせる。かいてんさせられる。",
            L"回転|される|。|回転|させる|。|回転|させ|られる|。");

    DoEntry(L"ぜったいします。ぜったいしてください。ぜったいにしないでください。");
    DoEntry(L"ぜったいするよ。ぜったいしたぞ。ぜったいできるな。ぜったいにしよう。");
    DoEntry(L"ぜったいしろ。ぜったいするな。ぜったいされる。");
    DoEntry(L"ぜったいさせる。ぜったいさせられる。",
            L"絶対|させる|。|絶対|させ|られる|。");
}

// 形容詞のテスト。
void DoKeiyoushi(void)
{
    DoEntry(L"すくない。すくなかろう。すくなかった。すくなく。すくなければ。",
            L"少ない|。|少なかろう|。|少なかった|。|少なく|。|少なければ|。");
    DoEntry(L"ただしい。ただしかろう。ただしかった。ただしく。ただしければ。",
            L"正しい|。|正しかろう|。|正しかった|。|正しく|。|正しければ|。");

    DoEntry(L"ゆたかだ。ゆたかだろう。ゆたかだった。ゆたかで。ゆたかに。ゆたかなこと。ゆたかならば。",
            L"豊かだ|。|豊かだろう|。|豊かだった|。|豊かで|。|豊かに|。|豊かな|こと|。|豊かならば|。");
}

// フレーズのテスト。
void DoPhrases(void)
{
    DoEntry(L"かのじょはにほんごがおじょうずですね。",
            L"彼女|は|日本語|が|お上手|ですね|。");
    DoEntry(L"わたしはしゅうきょうじょうのりゆうでおにくがたべられません。",
            L"私|は|宗教|上|の|理由|で|お肉|が|食べ|られ|ません|。");
    DoEntry(L"そこではなしはおわりになった", L"そこで|話|は|終わり|に|なった");
    DoEntry(L"わたしがわたしたわたをわたがしみたいにたべないでくださいませんか");
    DoEntry(L"あんた、そこにあいはあるんかいな");
    DoEntry(L"えがいたゆめはおおきかった。",
            L"描いた|夢|は|大きかった|。");
}

// mzimejaのテスト。
void IME_Test1(void)
{
    DoEntry(L"てすとです", L"テスト|です");

    DoEntry(L"たべないでください。");
    //DoDoushi();
    //DoKeiyoushi();
    //DoPhrases();
}

BOOL OnOK(HWND hwnd)
{
    WCHAR szText[1024];
    GetDlgItemTextW(hwnd, edt1, szText, _countof(szText));
    StrTrimW(szText, L" \t\r\n");
    if (szText[0] == 0) {
        MessageBoxW(hwnd, L"空ではない文字列を入力して下さい", NULL, 0);
        return FALSE;
    }
    DoEntry(szText, NULL, TRUE);
    return TRUE;
}

static INT_PTR CALLBACK
InputDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        SetDlgItemText(hwnd, edt1, L"かのじょはにほんごがおじょうずですね。");
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if (OnOK(hwnd)) {
                EndDialog(hwnd, IDOK);
            }
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
    return 0;
}

// ダイアログを用いてテストする。
void IME_Test2(void)
{
    while (::DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INPUTBOX),
                       NULL, InputDialogProc) == IDOK)
    {
        ;
    }
}

// Unicode版のmain関数。
int wmain(int argc, wchar_t **argv)
{
    // Unicode出力を可能に。
    std::setlocale(LC_CTYPE, "");

    LPCTSTR pathname = findLocalFile(L"res\\mzimeja.dic");
    //LPCTSTR pathname = findLocalFile(L"res\\testdata.dic");
    if (!g_basic_dict.Load(pathname, L"BasicDictObject")) {
        ASSERT(0);
        return 1;
    }

    // テスト1。
    IME_Test1();

    // テスト2。
    IME_Test2();

    g_basic_dict.Unload();

    return 0;
}

// 古いコンパイラのサポートのため。
int main(void)
{
    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    int ret = wmain(argc, argv);
    LocalFree(argv);
    return ret;
}
