#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <cctype>
#include <chrono>
#include <thread>
#include <ctime>
#include <vector>
#include <algorithm>
#include <csignal>
#include <cstring>

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
#else
  #include <termios.h>
  #include <unistd.h>
  #include <sys/select.h>
#endif

using namespace std;
using namespace std::chrono;

// ═══════════════════════════════════════════════════════════════════════
//  GEOMETRY — single source of truth
// ═══════════════════════════════════════════════════════════════════════
const int INDENT = 3;   // leading spaces on every visual element
const int BAR_W  = 58;  // width of every horizontal rule / fade bar
                        // total visible per row = INDENT + BAR_W = 61

// ═══════════════════════════════════════════════════════════════════════
//  ⚙  PRESET EVENT RATE
// ═══════════════════════════════════════════════════════════════════════
struct PresetRate {
    string eventName;
    string currencyName;
    string lastUpdated;
    double amountPerHour;
};

const PresetRate PRESET_RATE = {
    "Summer Festival 2025",
    "Candy",
    "2025-07-01",
    1500.0
};

struct UserCancelled {};

// ═══════════════════════════════════════════════════════════════════════
//  ANSI + truecolor
// ═══════════════════════════════════════════════════════════════════════
namespace A {
    const string RESET = "\033[0m", BOLD = "\033[1m", DIM = "\033[2m";
}
using namespace A;

struct RGB { int r, g, b; };
inline string fg(RGB c) {
    return "\033[38;2;" + to_string(c.r) + ";" + to_string(c.g) + ";" + to_string(c.b) + "m";
}
inline string bg(RGB c) {
    return "\033[48;2;" + to_string(c.r) + ";" + to_string(c.g) + ";" + to_string(c.b) + "m";
}
inline RGB lerp(RGB a, RGB b, double t) {
    t = t < 0 ? 0 : (t > 1 ? 1 : t);
    return { (int)(a.r + (b.r - a.r) * t + .5),
             (int)(a.g + (b.g - a.g) * t + .5),
             (int)(a.b + (b.b - a.b) * t + .5) };
}

const RGB CYAN_C   {   0, 230, 255 };
const RGB PINK_C   { 255,  20, 200 };
const RGB PURPLE_C { 160,  50, 255 };
const RGB GREEN_C  {  50, 255, 140 };
const RGB WHITE_C  { 240, 245, 255 };
const RGB GRAY_C   { 110, 115, 140 };
const RGB DARK_C   {  40,  44,  60 };
const RGB BLACK_C  {  15,  16,  24 };
const RGB RED_C    { 255,  80, 100 };

// ═══════════════════════════════════════════════════════════════════════
//  THEMES
// ═══════════════════════════════════════════════════════════════════════
struct Theme {
    RGB pulse1, pulse2;
    RGB accent;
    RGB title1, title2;
    RGB val1, val2;
};

const Theme THEME_BRAND = {
    {160,  50, 255}, {255,  20, 200},
    {  0, 230, 255},
    {  0, 230, 255}, {255,  20, 200},
    { 50, 255, 140}, {255, 220,  50}
};

const Theme THEME_RED = {
    {170,  30,  50}, {255, 100,  80},
    {220,  50,  70},
    {200,  30,  50}, {255, 110,  80},
    {255,  90,  60}, {255, 180, 130}
};

const Theme THEME_ORANGE = {
    {255,  80,  20}, {255, 180,  80},
    {255, 140,  30},
    {255, 100,  20}, {255, 210,  90},
    {255, 160,  50}, {255, 220, 130}
};

const Theme THEME_YELLOW = {
    {255, 255, 255}, {255, 230,  80},
    {240, 210,  50},
    {255, 255, 255}, {255, 220,  60},
    {255, 255, 255}, {255, 220,  60}
};

const Theme THEME_GREEN = {
    {  0, 120,  40}, { 60, 255, 120},
    { 40, 200,  90},
    {  0, 180,  60}, {100, 255, 140},
    { 50, 255, 120}, {150, 255, 180}
};

// ═══════════════════════════════════════════════════════════════════════
//  Text helpers
// ═══════════════════════════════════════════════════════════════════════
inline string repeatStr(const string& s, int n) {
    string r; r.reserve(s.size() * (size_t)(n > 0 ? n : 0));
    for (int i = 0; i < n; ++i) r += s;
    return r;
}

vector<string> splitChars(const string& s) {
    vector<string> out;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = s[i];
        size_t len = 1;
        if      ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        out.push_back(s.substr(i, len));
        i += len;
    }
    return out;
}

string osc8(const string& url, const string& text) {
    return "\033]8;;" + url + "\033\\" + text + "\033]8;;\033\\";
}

size_t visibleLen(const string& s) {
    size_t n = 0, i = 0;
    while (i < s.size()) {
        unsigned char c = s[i];
        if (c == 0x1B) {
            if (i + 1 < s.size() && s[i + 1] == '[') {
                i += 2;
                while (i < s.size()) {
                    unsigned char f = s[i++];
                    if (f >= 0x40 && f <= 0x7E) break;
                }
                continue;
            }
            if (i + 1 < s.size() && s[i + 1] == ']') {
                i += 2;
                while (i < s.size()) {
                    if (s[i] == 0x07) { i++; break; }
                    if (s[i] == 0x1B && i + 1 < s.size() && s[i + 1] == '\\') { i += 2; break; }
                    i++;
                }
                continue;
            }
            i++;
            continue;
        }
        if ((c & 0xC0) != 0x80) n++;
        i++;
    }
    return n;
}

string gradientLine(const string& line, RGB c1, RGB c2, bool bold = false) {
    auto chars = splitChars(line);
    int n = (int)chars.size();
    string out = bold ? BOLD : "";
    for (int i = 0; i < n; ++i) {
        double t = n > 1 ? (double)i / (n - 1) : 0;
        out += fg(lerp(c1, c2, t)) + chars[i];
    }
    return out + RESET;
}

string padRight(const string& s, int w) {
    int len = (int)visibleLen(s);
    if (len >= w) return s;
    return s + string((size_t)(w - len), ' ');
}

// ═══════════════════════════════════════════════════════════════════════
//  Cursor / screen — clears scrollback too
// ═══════════════════════════════════════════════════════════════════════
void clearScreen() { cout << "\033[2J\033[3J\033[H" << flush; }
void homeCursor()  { cout << "\033[H"    << flush; }
void hideCursor()  { cout << "\033[?25l" << flush; }
void showCursor()  { cout << "\033[?25h" << flush; }
void clearLine()   { cout << "\033[2K\r" << flush; }

// ═══════════════════════════════════════════════════════════════════════
//  Animation
// ═══════════════════════════════════════════════════════════════════════
void typewrite(const string& s, int delayMs = 16) {
    for (auto& ch : splitChars(s)) {
        cout << ch << flush;
        if (delayMs > 0) this_thread::sleep_for(milliseconds(delayMs));
    }
}

void spinnerTask(const string& label, int ms, RGB col = CYAN_C) {
    static const char* frames[] = {"⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏"};
    auto start = steady_clock::now();
    int i = 0;
    while (duration_cast<milliseconds>(steady_clock::now() - start).count() < ms) {
        clearLine();
        cout << string(INDENT, ' ') << fg(col) << frames[i++ % 10] << RESET
             << " " << fg(WHITE_C) << label << RESET << flush;
        this_thread::sleep_for(milliseconds(60));
    }
    clearLine();
    cout << string(INDENT, ' ') << fg(GREEN_C) << "✔" << RESET
         << " " << fg(WHITE_C) << label << RESET << "\n" << flush;
}

void progressTask(const string& label, int ms, int width = 38) {
    auto start = steady_clock::now();
    while (true) {
        auto el = duration_cast<milliseconds>(steady_clock::now() - start).count();
        double pct = el >= ms ? 1.0 : (double)el / ms;
        int filled = (int)(pct * width);
        clearLine();
        cout << string(INDENT, ' ') << fg(WHITE_C) << padRight(label, 18) << RESET << " "
             << fg(GRAY_C) << "[";
        for (int i = 0; i < width; ++i)
            cout << (i < filled ? fg(lerp(CYAN_C, PINK_C, (double)i / (width - 1))) + "█"
                                : fg(DARK_C) + "░");
        cout << fg(GRAY_C) << "]" << fg(WHITE_C)
             << " " << setw(3) << (int)(pct * 100) << "%" << RESET << flush;
        if (pct >= 1.0) break;
        this_thread::sleep_for(milliseconds(20));
    }
    cout << "\n";
}

// Every fade bar: INDENT spaces + up to BAR_W glyphs, \033[K clears trailing.
void introWipe() {
    for (int i = 0; i <= BAR_W; i += 8) {
        int w = min(i, BAR_W);
        cout << "\r" << string(INDENT, ' ')
             << fg(lerp(CYAN_C, PINK_C, (double)w / BAR_W))
             << repeatStr("▔", w) << "\033[K" << RESET << flush;
        this_thread::sleep_for(milliseconds(6));
    }
    cout << "\n" << flush;
}

// ═══════════════════════════════════════════════════════════════════════
//  Raw mode
// ═══════════════════════════════════════════════════════════════════════
struct RawMode {
    bool on = false;
#ifndef _WIN32
    struct termios oldT{};
#endif
    void enable() {
        if (on) return;
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
        DWORD m; GetConsoleMode(h, &m);
        SetConsoleMode(h, m & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT));
#else
        tcgetattr(0, &oldT);
        struct termios t = oldT;
        t.c_lflag &= ~(ICANON | ECHO);
        t.c_cc[VMIN] = 0; t.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &t);
#endif
        on = true;
    }
    void disable() {
        if (!on) return;
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
        DWORD m; GetConsoleMode(h, &m);
        SetConsoleMode(h, m | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
#else
        tcsetattr(0, TCSANOW, &oldT);
#endif
        on = false;
    }
    ~RawMode() { disable(); }
};
RawMode g_raw;

void restoreTerminal() { g_raw.disable(); cout << "\033[?25h" << RESET << flush; }
#ifdef _WIN32
BOOL WINAPI ctrlHandler(DWORD) { restoreTerminal(); return FALSE; }
#else
void signalHandler(int) {
    const char* s = "\033[?25h\033[0m\n";
    (void)!write(1, s, strlen(s));
    _exit(130);
}
#endif

int readKey() {
#ifdef _WIN32
    int c = _getch();
    if (c == 0 || c == 0xE0) {
        int c2 = _getch();
        if (c2 == 72) return 'U';
        if (c2 == 80) return 'D';
        if (c2 == 75) return 'L';
        if (c2 == 77) return 'R';
        return 0;
    }
    return c;
#else
    unsigned char c;
    if (read(0, &c, 1) != 1) return 0;
    if (c == 27) {
        fd_set fds; FD_ZERO(&fds); FD_SET(0, &fds);
        struct timeval tv{0, 30000};
        if (select(1, &fds, nullptr, nullptr, &tv) > 0) {
            unsigned char c2, c3;
            if (read(0, &c2, 1) != 1) return 27;
            if (c2 == '[' && read(0, &c3, 1) == 1) {
                if (c3 == 'A') return 'U';
                if (c3 == 'B') return 'D';
                if (c3 == 'C') return 'R';
                if (c3 == 'D') return 'L';
            }
        }
        return 27;
    }
    return c;
#endif
}

bool keyAvailable(int ms) {
#ifdef _WIN32
    auto start = steady_clock::now();
    while (duration_cast<milliseconds>(steady_clock::now() - start).count() < ms) {
        if (_kbhit()) return true;
        this_thread::sleep_for(milliseconds(15));
    }
    return false;
#else
    fd_set fds; FD_ZERO(&fds); FD_SET(0, &fds);
    struct timeval tv;
    tv.tv_sec = ms / 1000; tv.tv_usec = (ms % 1000) * 1000;
    return select(1, &fds, nullptr, nullptr, &tv) > 0;
#endif
}

string readLineEsc(const string& prompt, RGB col = CYAN_C) {
    bool wasOn = g_raw.on;
    g_raw.enable();
    showCursor();

    string buffer;
    bool cancelled = false;

    auto redraw = [&]() {
        cout << "\r" << fg(col) << prompt << RESET << buffer << "\033[K" << flush;
    };
    redraw();

    while (true) {
        if (!keyAvailable(50)) continue;
        int k = readKey();
        if (k == 27) { cancelled = true; break; }
        if (k == '\n' || k == '\r') break;
        if (k == 127 || k == 8) {
            if (!buffer.empty()) {
                size_t lastByte = buffer.size() - 1;
                while (lastByte > 0 && ((unsigned char)buffer[lastByte] & 0xC0) == 0x80) lastByte--;
                buffer.erase(lastByte);
                redraw();
            }
            continue;
        }
        if (k >= 32 && k < 127) {
            buffer += (char)k;
            redraw();
        }
    }

    cout << "\n" << flush;
    hideCursor();
    if (!wasOn) g_raw.disable();

    if (cancelled) throw UserCancelled{};
    return buffer;
}

void pauseAny() {
    g_raw.enable();
    cout << "\n" << string(INDENT, ' ') << fg(GRAY_C) << DIM
         << "press any key to continue…" << RESET << flush;
    while (!keyAvailable(100)) {}
    readKey();
    g_raw.disable();
    cout << "\n";
}

// ═══════════════════════════════════════════════════════════════════════
//  URL opener
// ═══════════════════════════════════════════════════════════════════════
bool openUrl(const string& url) {
#ifdef _WIN32
    string cmd = "start \"\" \"" + url + "\" >nul 2>&1";
#elif defined(__APPLE__)
    string cmd = "open \"" + url + "\" </dev/null >/dev/null 2>&1";
#else
    string cmd = "xdg-open \"" + url + "\" </dev/null >/dev/null 2>&1 &";
#endif
    int rc = system(cmd.c_str());
    return rc == 0;
}

// ═══════════════════════════════════════════════════════════════════════
//  Numbers & clock
// ═══════════════════════════════════════════════════════════════════════
string fmtNum(double v) {
    bool neg = v < 0; double a = fabs(v);
    double r = floor(a * 100.0 + 0.5) / 100.0;
    long long ip = (long long)floor(r);
    int fp = (int)llround((r - (double)ip) * 100.0);
    string s = to_string(ip);
    for (int i = (int)s.size() - 3; i > 0; i -= 3) s.insert((size_t)i, ",");
    if (fp != 0) {
        ostringstream os; os << '.' << setw(2) << setfill('0') << fp;
        s += os.str();
    }
    return (neg ? "-" : "") + s;
}

string currentClock() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buf[16]; strftime(buf, sizeof(buf), "%H:%M:%S", t);
    return buf;
}

// ═══════════════════════════════════════════════════════════════════════
//  Logo
// ═══════════════════════════════════════════════════════════════════════
const vector<string> LOGO = {
    "░█████╗░███╗░░░███╗████████╗░█████╗░░█████╗░██╗░░░░░",
    "██╔══██╗████╗░████║╚══██╔══╝██╔══██╗██╔══██╗██║░░░░░",
    "███████║██╔████╔██║░░░██║░░░██║░░██║██║░░██║██║░░░░░",
    "██╔══██║██║╚██╔╝██║░░░██║░░░██║░░██║██║░░██║██║░░░░░",
    "██║░░██║██║░╚═╝░██║░░░██║░░░╚█████╔╝╚█████╔╝███████╗",
    "╚═╝░░╚═╝╚═╝░░░░░╚═╝░░░╚═╝░░░░╚════╝░░╚════╝░╚══════╝",
};

void revealLogo() {
    for (auto& line : LOGO) {
        auto chars = splitChars(line);
        int n = (int)chars.size();
        cout << string(INDENT, ' ');
        for (int i = 0; i < n; ++i) {
            double t = n > 1 ? (double)i / (n - 1) : 0;
            cout << fg(lerp(CYAN_C, PINK_C, t)) << chars[i] << flush;
            this_thread::sleep_for(milliseconds(2));
        }
        cout << RESET << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Panel chrome — INDENT + ╭ + PANEL_W + ╮ = INDENT + 2 + PANEL_W
//  To match BAR_W: PANEL_W must = BAR_W - 2 = 56.
// ═══════════════════════════════════════════════════════════════════════
const int PANEL_W = BAR_W - 2;   // 56

void panelTop(const string& title, RGB accent) {
    if (title.empty()) {
        cout << string(INDENT, ' ') << fg(accent)
             << "╭" << repeatStr("─", PANEL_W) << "╮" << RESET << "\n";
        return;
    }
    string t = " " + title + " ";
    int tl = (int)visibleLen(t);
    int left = 2, right = PANEL_W - left - tl;
    if (right < 0) right = 0;
    cout << string(INDENT, ' ') << fg(accent)
         << "╭" << repeatStr("─", left) << RESET
         << fg(accent) << BOLD << t << RESET
         << fg(accent) << repeatStr("─", right) << "╮" << RESET << "\n";
}

void panelLine(const string& content, RGB accent) {
    int len = (int)visibleLen(content);
    int pad = PANEL_W - len;
    if (pad < 0) pad = 0;
    cout << string(INDENT, ' ') << fg(accent) << "│" << RESET
         << content << string((size_t)pad, ' ')
         << fg(accent) << "│" << RESET << "\n";
}

void panelSep(RGB accent) {
    cout << string(INDENT, ' ') << fg(accent)
         << "├" << repeatStr("─", PANEL_W) << "┤" << RESET << "\n";
}
void panelSepThick(RGB accent) {
    cout << string(INDENT, ' ') << fg(accent)
         << "╞" << repeatStr("═", PANEL_W) << "╡" << RESET << "\n";
}
void panelBottom(RGB accent) {
    cout << string(INDENT, ' ') << fg(accent)
         << "╰" << repeatStr("─", PANEL_W) << "╯" << RESET << "\n";
}

// ═══════════════════════════════════════════════════════════════════════
//  FormPanel
// ═══════════════════════════════════════════════════════════════════════
struct FormPanel {
    string title;
    string subtitle;
    Theme theme;
    vector<pair<string,string>> qa;

    FormPanel(const string& t, const Theme& th, const string& s = "")
        : title(t), subtitle(s), theme(th) {}

    void draw() {
        RGB ac = theme.accent;
        homeCursor();
        cout << "\033[2K\n\n\n\n";   // 4 blank lines of breathing room

        cout << "\033[2K"; panelTop(title, ac);
        cout << "\033[2K"; panelLine("", ac);

        if (!subtitle.empty()) {
            cout << "\033[2K";
            panelLine("  " + fg(GRAY_C) + DIM + subtitle + RESET, ac);
            cout << "\033[2K"; panelLine("", ac);
        }

        if (!qa.empty()) {
            cout << "\033[2K"; panelSep(ac);
            cout << "\033[2K"; panelLine("", ac);
        }

        for (auto& p : qa) {
            if (p.first.empty() && p.second.empty()) {
                cout << "\033[2K"; panelSep(ac);
                continue;
            }
            if (p.first == "=" && p.second.empty()) {
                cout << "\033[2K"; panelSepThick(ac);
                continue;
            }
            if (p.first == "—") {
                cout << "\033[2K";
                panelLine("  " + fg(ac) + BOLD + p.second + RESET, ac);
                continue;
            }

            bool lblColored = p.first.find("\033[")  != string::npos;
            bool valColored = p.second.find("\033[") != string::npos;

            string lbl = "  ";
            if (lblColored) lbl += padRight(p.first, 22);
            else            lbl += fg(GRAY_C) + padRight(p.first, 22) + RESET;

            string val;
            if (valColored) val = p.second;
            else            val = fg(WHITE_C) + BOLD + p.second + RESET;

            cout << "\033[2K";
            panelLine(lbl + " " + val, ac);
        }

        cout << "\033[2K"; panelLine("", ac);
        cout << "\033[2K"; panelBottom(ac);
        cout << "\033[J";
        cout.flush();
    }

    void sep()  { qa.push_back({"", ""}); }
    void thick(){ qa.push_back({"=", ""}); }
    void hdr(const string& t) { qa.push_back({"—", t}); }

    void addGrad(const string& label, const string& value) {
        qa.push_back({
            gradientLine(label, theme.title1, theme.title2, true),
            gradientLine(value, theme.val1, theme.val2, true)
        });
    }

    void promptQuestion(const string& q) {
        draw();
        cout << "\n" << string(INDENT, ' ') << fg(theme.pulse2) << BOLD << "▸ " << q << RESET << "\n";
    }

    string askStr(const string& question, const string& label,
                  const string& def = "") {
        promptQuestion(question);
        string ans = readLineEsc(string(INDENT + 2, ' ') + "› ", theme.pulse2);
        if (ans.empty()) ans = def;
        qa.push_back({ label, ans.empty() ? "—" : ans });
        return ans;
    }

    bool askAuto(const string& question, const string& label) {
        promptQuestion(question);
        string s = readLineEsc(string(INDENT + 2, ' ') + "› ", theme.pulse2);
        bool yes = !s.empty();
        qa.push_back({ label, yes ? "yes" : "no" });
        return yes;
    }

    double askNum(const string& question, const string& label,
                  double mn = -1e300, double mx = 1e300) {
        promptQuestion(question);
        while (true) {
            string s = readLineEsc(string(INDENT + 2, ' ') + "› ", theme.pulse2);
            try {
                size_t pos; double v = stod(s, &pos);
                if (pos != s.size() || !isfinite(v) || v < mn || v > mx) throw runtime_error("");
                qa.push_back({ label, fmtNum(v) });
                return v;
            } catch (...) {
                cout << string(INDENT + 2, ' ') << fg(RED_C) << "✖ invalid number\n" << RESET;
            }
        }
    }

    bool askNumOpt(const string& question, const string& label,
                   double& out, double mn = 0, double mx = 1e300) {
        promptQuestion(question);
        while (true) {
            string s = readLineEsc(string(INDENT + 2, ' ') + "› ", theme.pulse2);
            if (s.empty()) {
                qa.push_back({ label, "—" });
                return false;
            }
            try {
                size_t pos; double v = stod(s, &pos);
                if (pos != s.size() || !isfinite(v) || v < mn || v > mx) throw runtime_error("");
                out = v;
                qa.push_back({ label, fmtNum(v) });
                return true;
            } catch (...) {
                cout << string(INDENT + 2, ' ') << fg(RED_C) << "✖ invalid number\n" << RESET;
            }
        }
    }

    long long askInt(const string& question, const string& label,
                     long long mn = LLONG_MIN, long long mx = LLONG_MAX) {
        promptQuestion(question);
        while (true) {
            string s = readLineEsc(string(INDENT + 2, ' ') + "› ", theme.pulse2);
            try {
                size_t pos; long long v = stoll(s, &pos);
                if (pos != s.size() || v < mn || v > mx) throw runtime_error("");
                qa.push_back({ label, to_string(v) });
                return v;
            } catch (...) {
                cout << string(INDENT + 2, ' ') << fg(RED_C) << "✖ invalid integer\n" << RESET;
            }
        }
    }

    long long askIntDefault(const string& question, const string& label,
                            long long def,
                            long long mn = LLONG_MIN, long long mx = LLONG_MAX) {
        promptQuestion(question);
        while (true) {
            string s = readLineEsc(string(INDENT + 2, ' ') + "› ", theme.pulse2);
            if (s.empty()) {
                qa.push_back({ label, to_string(def) });
                return def;
            }
            try {
                size_t pos; long long v = stoll(s, &pos);
                if (pos != s.size() || v < mn || v > mx) throw runtime_error("");
                qa.push_back({ label, to_string(v) });
                return v;
            } catch (...) {
                cout << string(INDENT + 2, ' ') << fg(RED_C) << "✖ invalid integer\n" << RESET;
            }
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════
//  Screen transition — consistent bar width, erased frames, breathing room
// ═══════════════════════════════════════════════════════════════════════
void screenSwipe(const string& next, const Theme& theme) {
    clearScreen();

    // fade in
    for (int i = 0; i <= BAR_W; i += 8) {
        int w = min(i, BAR_W);
        cout << "\r" << string(INDENT, ' ')
             << fg(lerp(theme.accent, theme.pulse2, (double)w / BAR_W))
             << repeatStr("▔", w) << "\033[K" << RESET << flush;
        this_thread::sleep_for(milliseconds(4));
    }
    cout << "\n\n\n";

    // spinner + loading text
    static const char* frames[] = {"⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏"};
    auto t0 = steady_clock::now();
    int fi = 0;
    while (duration_cast<milliseconds>(steady_clock::now() - t0).count() < 130) {
        cout << "\r" << string(INDENT, ' ') << fg(theme.accent) << frames[fi++ % 10] << RESET
             << " " << fg(GRAY_C) << DIM << "loading " << RESET
             << fg(WHITE_C) << next << RESET << "\033[K" << flush;
        this_thread::sleep_for(milliseconds(40));
    }
    cout << "\r" << string(INDENT, ' ') << fg(theme.pulse2) << "✔" << RESET
         << " " << fg(GRAY_C) << DIM << "loading " << RESET
         << fg(WHITE_C) << next << RESET << "\033[K" << flush;
    this_thread::sleep_for(milliseconds(60));
    cout << "\n\n\n";

    // fade out
    for (int i = BAR_W; i >= 0; i -= 8) {
        int w = max(0, i);
        cout << "\r" << string(INDENT, ' ')
             << fg(lerp(theme.accent, theme.pulse2, (double)w / BAR_W))
             << repeatStr("▁", w) << "\033[K" << RESET << flush;
        this_thread::sleep_for(milliseconds(4));
    }
    cout << "\n";
    this_thread::sleep_for(milliseconds(50));
    clearScreen();
}

// ═══════════════════════════════════════════════════════════════════════
//  Generic select menu
// ═══════════════════════════════════════════════════════════════════════
struct MenuEntry { string key, title, sub; char shortcut; };

int runSelectMenu(const string& title, const vector<MenuEntry>& items,
                  const Theme& theme, const string& label = "",
                  const string& escHint = "esc back") {
    bool wasOn = g_raw.on;
    g_raw.enable();
    hideCursor();

    int sel = 0;
    auto pulseBase = steady_clock::now();
    int result = -1;

    while (true) {
        auto el = duration_cast<milliseconds>(steady_clock::now() - pulseBase).count();
        double phase = (sin(el * 0.0032) + 1.0) * 0.5;
        RGB hl = lerp(theme.pulse1, theme.pulse2, phase);

        homeCursor();
        cout << "\033[2K\n\n\n\n";   // 4 blank lines of breathing room

        // header
        cout << "\033[2K" << string(INDENT, ' ')
             << gradientLine(title, theme.title1, theme.title2, true);
        if (!label.empty()) cout << "  " << fg(GRAY_C) << DIM << label << RESET;
        cout << "\n";
        cout << "\033[2K" << string(INDENT, ' ')
             << fg(theme.accent) << repeatStr("─", BAR_W) << RESET << "\n";
        cout << "\033[2K\n";

        // items
        for (int i = 0; i < (int)items.size(); ++i) {
            const auto& e = items[i];
            bool active = (i == sel);

            cout << "\033[2K";
            if (active) {
                string content = "  ›  [" + e.key + "]  " + e.title;
                int pad = BAR_W - (int)visibleLen(content);
                if (pad < 0) pad = 0;
                cout << string(INDENT, ' ') << bg(hl) << fg(BLACK_C) << BOLD
                     << content << string((size_t)pad, ' ') << RESET;
            } else {
                string content = "     [" + e.key + "]  " + e.title;
                int pad = BAR_W - (int)visibleLen(content);
                if (pad < 0) pad = 0;
                cout << string(INDENT, ' ') << fg(GRAY_C) << "[" << e.key << "]  " << RESET
                     << fg(WHITE_C) << e.title << RESET
                     << string((size_t)pad, ' ');
            }
            cout << "\n";

            if (!e.sub.empty()) {
                cout << "\033[2K" << string(INDENT + 7, ' ')
                     << (active ? fg(hl) + DIM : fg(DARK_C))
                     << e.sub << RESET << "\n";
            }
            cout << "\033[2K\n";
        }

        // footer
        cout << "\033[2K" << string(INDENT, ' ')
             << fg(theme.accent) << repeatStr("─", BAR_W) << RESET << "\n";
        cout << "\033[2K" << string(INDENT, ' ')
             << fg(GRAY_C) << DIM << "↑↓ navigate   ⏎ select   " << escHint << "   " << RESET
             << fg(theme.accent) << "● " << fg(WHITE_C) << currentClock() << RESET
             << "\n";
        cout << "\033[J" << flush;

        if (!keyAvailable(45)) continue;
        int k = readKey();
        if (k == 'U') sel = (sel - 1 + (int)items.size()) % (int)items.size();
        else if (k == 'D') sel = (sel + 1) % (int)items.size();
        else if (k == '\n' || k == '\r') { result = sel; break; }
        else if (k == 27) { result = -1; break; }
        else {
            bool found = false;
            for (int i = 0; i < (int)items.size(); ++i) {
                if (tolower(k) == items[i].shortcut) { result = i; found = true; break; }
            }
            if (found) break;
        }
    }

    hideCursor();
    if (!wasOn) g_raw.disable();
    return result;
}

// ═══════════════════════════════════════════════════════════════════════
//  Boot — logo, tagline, wipe, spinners, module progress, ready
// ═══════════════════════════════════════════════════════════════════════
void bootSequence() {
    clearScreen();
    hideCursor();
    cout << "\n\n";
    revealLogo();
    cout << "\n" << string(INDENT, ' ') << fg(GRAY_C) << DIM;
    typewrite("Adopt Me! · calculator suite · by @wtvsad", 12);
    cout << RESET << "\n\n";
    this_thread::sleep_for(milliseconds(280));
    introWipe();
    cout << "\n";

    spinnerTask("TUI engine online",        480);
    spinnerTask("truecolor palette loaded", 380);
    spinnerTask("render pipeline ready",    460);
    spinnerTask("input subsystem armed",    300);
    cout << "\n";
    progressTask("module  nmcost  ", 520);
    progressTask("module  timecalc", 480);
    progressTask("module  probcalc", 520);

    cout << "\n" << string(INDENT, ' ') << fg(GREEN_C) << "●" << RESET << " "
         << fg(WHITE_C) << "System ready." << RESET << "\n\n";
    this_thread::sleep_for(milliseconds(400));
}

// ═══════════════════════════════════════════════════════════════════════
//  pet accumulator
// ═══════════════════════════════════════════════════════════════════════
struct PetLine {
    string name;
    double unit;
    long long nN = 0, nNeon = 0, nMega = 0;
    long long totalPets() const { return nN + nNeon * 4 + nMega * 16; }
    double totalCost() const { return unit * (double)totalPets(); }
};

// ═══════════════════════════════════════════════════════════════════════
//  NMCOST · flexible
// ═══════════════════════════════════════════════════════════════════════
void toolFlexibleCost() {
    screenSwipe("NMCOST · flexible", THEME_RED);
    FormPanel fp("COST CALCULATOR · FLEXIBLE", THEME_RED,
                 "Add the cost of any number of pets.");

    string curName = fp.askStr("Currency name (blank = none)", "currency");
    string cur = curName.empty() ? "" : " " + curName;

    double grand = 0;
    int idx = 1;
    while (true) {
        fp.sep();
        fp.hdr("PET #" + to_string(idx));

        string name = fp.askStr(
            "Pet #" + to_string(idx) + " name (blank = 'pet')", "name", "pet");
        if (name.empty()) name = "pet";

        double cost = fp.askNum("Unit cost of " + name, "unit cost");
        long long qty = fp.askInt("Quantity of " + name, "quantity", 0);
        double total = cost * (double)qty;
        grand += total;
        fp.qa.push_back({ "subtotal", fmtNum(total) + cur });

        if (!fp.askAuto("Add another pet?", "another pet?")) break;
        ++idx;
    }

    fp.thick();
    fp.addGrad("GRAND TOTAL", fmtNum(grand) + cur);
    fp.thick();
    fp.draw();
    pauseAny();
}

// ═══════════════════════════════════════════════════════════════════════
//  NMCOST · NEON / MEGA
// ═══════════════════════════════════════════════════════════════════════
void toolNeonMega() {
    screenSwipe("NMCOST · NEON/MEGA", THEME_RED);
    FormPanel fp("COST CALCULATOR · NEON / MEGA", THEME_RED,
                 "NEON = 4 pets  ·  MEGA = 16 pets");

    string curName = fp.askStr("Currency name (blank = none)", "currency");
    string cur = curName.empty() ? "" : " " + curName;

    vector<PetLine> pets;
    int idx = 1;

    while (true) {
        fp.sep();
        fp.hdr("PET #" + to_string(idx));

        string name = fp.askStr(
            "Pet #" + to_string(idx) + " name (blank = 'pet')", "name", "pet");
        if (name.empty()) name = "pet";

        double unit = fp.askNum("Unit cost of " + name, "unit cost");

        fp.sep();
        fp.hdr("COST TABLE · " + name);
        fp.qa.push_back({ "1 pet",             fmtNum(unit)      + cur });
        fp.qa.push_back({ "1 NEON   ×4",       fmtNum(unit * 4)  + cur });
        fp.qa.push_back({ "1 MEGA   ×16",      fmtNum(unit * 16) + cur });
        fp.qa.push_back({ "1 NEON + 1 MEGA",   fmtNum(unit * 20) + cur });
        fp.qa.push_back({ "all 3  n+N+M  ×21", fmtNum(unit * 21) + cur });

        fp.sep();
        fp.hdr("ADD TO TOTAL · " + name);
        PetLine pl; pl.name = name; pl.unit = unit;
        pl.nN    = fp.askIntDefault("Normal " + name + " count (blank = 1)", "normal",  1, 0);
        pl.nNeon = fp.askIntDefault("NEON   " + name + " count (blank = 0)", "NEON",    0, 0);
        pl.nMega = fp.askIntDefault("MEGA   " + name + " count (blank = 0)", "MEGA",    0, 0);

        long long totalPets = pl.totalPets();
        fp.qa.push_back({ "pets total", fmtNum((double)totalPets) });
        fp.qa.push_back({ "subtotal",   fmtNum(pl.totalCost()) + cur });

        pets.push_back(pl);

        if (!fp.askAuto("Add another pet?", "another pet?")) break;
        ++idx;
    }

    double sumUnit = 0;
    for (auto& p : pets) sumUnit += p.unit;

    auto V = [&](double v) {
        return gradientLine(fmtNum(v) + cur, THEME_RED.val1, THEME_RED.val2, true);
    };

    fp.thick();
    fp.qa.push_back({
        gradientLine("GRAND TOTAL · 1 OF EACH PET",
                     THEME_RED.title1, THEME_RED.title2, true), ""
    });
    fp.qa.push_back({ "1 normal",        V(sumUnit)      });
    fp.qa.push_back({ "1 NEON  ×4",      V(sumUnit * 4)  });
    fp.qa.push_back({ "1 MEGA  ×16",     V(sumUnit * 16) });
    fp.qa.push_back({ "1 NEON + 1 MEGA", V(sumUnit * 20) });
    fp.qa.push_back({ "all 3 (n+N+M)",   V(sumUnit * 21) });

    fp.thick();
    fp.qa.push_back({
        gradientLine("CUSTOM BREAKDOWN",
                     THEME_RED.title1, THEME_RED.title2, true), ""
    });
    double grand = 0;
    for (auto& p : pets) {
        double tc = p.totalCost();
        grand += tc;
        string brk = to_string(p.nN) + "n·" + to_string(p.nNeon) + "N·"
                   + to_string(p.nMega) + "M = "
                   + fmtNum((double)p.totalPets()) + " pets";
        fp.qa.push_back({ p.name, brk });
        fp.qa.push_back({ "", "→ " + fmtNum(tc) + cur });
    }
    fp.thick();
    fp.addGrad("CUSTOM TOTAL", fmtNum(grand) + cur);
    fp.thick();

    fp.draw();
    pauseAny();
}

// ═══════════════════════════════════════════════════════════════════════
//  NMCOST · sub-menu
// ═══════════════════════════════════════════════════════════════════════
void toolNmcostMenu() {
    vector<MenuEntry> items = {
        { "a", "flexible cost",  "any amount of pets",  'a' },
        { "b", "NEON / MEGA",    "×4 / ×16 / custom",   'b' },
    };
    while (true) {
        screenSwipe("NMCOST", THEME_RED);
        int idx = runSelectMenu("COST CALCULATOR", items, THEME_RED, "red");
        if (idx < 0) return;
        try {
            if (idx == 0) toolFlexibleCost();
            else if (idx == 1) toolNeonMega();
        } catch (UserCancelled&) {}
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  TIMECALC · shared result renderer
// ═══════════════════════════════════════════════════════════════════════
void timecalcFinish(FormPanel& fp, double perHour, const string& cur) {
    if (perHour <= 0) {
        fp.sep();
        fp.qa.push_back({ "result", "✖ rate must be > 0" });
        fp.draw(); pauseAny(); return;
    }

    double current = fp.askNum("Current amount", "current");
    double target  = fp.askNum("Target  amount", "target");

    if (target <= current) {
        fp.sep();
        fp.qa.push_back({ "result", "✔ already enough!" });
        fp.draw(); pauseAny(); return;
    }

    double need    = target - current;
    double hours   = need / perHour;
    long long H = (long long)floor(hours);
    long long M = (long long)llround((hours - (double)H) * 60.0);
    if (M >= 60) { H++; M -= 60; }
    long long D = H / 24, HH = H % 24;

    string eta;
    if (D > 0) eta += to_string(D) + "d ";
    eta += to_string(HH) + "h " + to_string(M) + "m";
    ostringstream os; os << fixed << setprecision(2) << hours;

    fp.sep();
    fp.qa.push_back({ "remaining", fmtNum(need) + cur });
    fp.qa.push_back({ "rate",      fmtNum(perHour) + cur + "/h" });
    fp.thick();
    fp.addGrad("ESTIMATED TIME", eta);
    fp.qa.push_back({ "total", os.str() + " hours" });
    fp.thick();
    fp.draw();
    pauseAny();
}

// ═══════════════════════════════════════════════════════════════════════
//  TIMECALC · quick
// ═══════════════════════════════════════════════════════════════════════
void toolTimecalcQuick() {
    screenSwipe("TIMECALC · QUICK", THEME_ORANGE);
    FormPanel fp("QUICK TIME CALCULATOR", THEME_ORANGE,
                 "rate preset by the tool dev");

    if (PRESET_RATE.eventName.empty() || PRESET_RATE.amountPerHour <= 0) {
        fp.sep();
        fp.qa.push_back({ "result", "✖ no preset rate set — use custom" });
        fp.draw(); pauseAny(); return;
    }

    fp.sep();
    fp.hdr("PRESET RATE");
    fp.qa.push_back({ "event",        PRESET_RATE.eventName });
    fp.qa.push_back({ "last updated", PRESET_RATE.lastUpdated });

    fp.thick();
    fp.addGrad("RATE",
               fmtNum(PRESET_RATE.amountPerHour) + " " + PRESET_RATE.currencyName + "/h");
    fp.thick();

    string cur = " " + PRESET_RATE.currencyName;
    timecalcFinish(fp, PRESET_RATE.amountPerHour, cur);
}

// ═══════════════════════════════════════════════════════════════════════
//  TIMECALC · custom · enter rate
// ═══════════════════════════════════════════════════════════════════════
void toolTimecalcCustomRate() {
    screenSwipe("TIMECALC · CUSTOM · RATE", THEME_ORANGE);
    FormPanel fp("TIME CALCULATOR · CUSTOM", THEME_ORANGE,
                 "enter your rate directly");

    string curName = fp.askStr("Currency name (blank = none)", "currency");
    string cur = curName.empty() ? "" : " " + curName;

    char unit;
    do {
        fp.promptQuestion("Rate unit — h(our) or m(inute)");
        string s = readLineEsc(string(INDENT + 2, ' ') + "› ", THEME_ORANGE.pulse2);
        unit = s.empty() ? '?' : (char)tolower((unsigned char)s[0]);
    } while (unit != 'h' && unit != 'm');
    fp.qa.push_back({ "unit", unit == 'h' ? "per hour" : "per minute" });

    double rate = fp.askNum(
        unit == 'h' ? "Amount earned per hour" : "Amount earned per minute",
        "rate", 1e-9);
    double perHour = unit == 'h' ? rate : rate * 60.0;

    timecalcFinish(fp, perHour, cur);
}

// ═══════════════════════════════════════════════════════════════════════
//  TIMECALC · custom · derive rate from session
// ═══════════════════════════════════════════════════════════════════════
void toolTimecalcCustomEarnings() {
    screenSwipe("TIMECALC · CUSTOM · EARNINGS", THEME_ORANGE);
    FormPanel fp("TIME CALCULATOR · CUSTOM", THEME_ORANGE,
                 "derive rate from a past session");

    string curName = fp.askStr("Currency name (blank = none)", "currency");
    string cur = curName.empty() ? "" : " " + curName;

    long long hoursPlayed   = fp.askInt("Hours played in past session", "hours", 0);
    long long minutesPlayed = fp.askInt("Extra minutes (0-59)", "minutes", 0, 59);
    double amountEarned     = fp.askNum("Total amount earned during that time", "earned", 0);

    double totalMinutes = (double)(hoursPlayed * 60 + minutesPlayed);
    if (totalMinutes <= 0 || amountEarned <= 0) {
        fp.sep();
        fp.qa.push_back({ "result", "✖ time and earnings must be > 0" });
        fp.draw(); pauseAny(); return;
    }

    double totalHours = totalMinutes / 60.0;
    double perHour    = amountEarned / totalHours;

    fp.sep();
    fp.hdr("DERIVED RATE");
    fp.qa.push_back({ "session", to_string(hoursPlayed) + "h " + to_string(minutesPlayed) + "m" });
    fp.thick();
    fp.addGrad("YOUR RATE", fmtNum(perHour) + cur + "/h");
    fp.thick();

    timecalcFinish(fp, perHour, cur);
}

// ═══════════════════════════════════════════════════════════════════════
//  TIMECALC · custom sub-menu
// ═══════════════════════════════════════════════════════════════════════
void toolTimecalcCustom() {
    vector<MenuEntry> items = {
        { "a", "I know my rate",     "type it in",               'a' },
        { "b", "I know my earnings", "derive from past session", 'b' },
    };
    while (true) {
        screenSwipe("TIMECALC · CUSTOM", THEME_ORANGE);
        int idx = runSelectMenu("CUSTOM TIME CALCULATOR", items, THEME_ORANGE, "orange");
        if (idx < 0) return;
        try {
            if (idx == 0) toolTimecalcCustomRate();
            else if (idx == 1) toolTimecalcCustomEarnings();
        } catch (UserCancelled&) {}
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  TIMECALC · main sub-menu
// ═══════════════════════════════════════════════════════════════════════
void toolTimecalc() {
    vector<MenuEntry> items = {
        { "a", "quick",   "use preset event rate",   'a' },
        { "b", "custom",  "enter rate or earnings",  'b' },
    };
    while (true) {
        screenSwipe("TIMECALC", THEME_ORANGE);
        int idx = runSelectMenu("TIME CALCULATOR", items, THEME_ORANGE, "orange");
        if (idx < 0) return;
        try {
            if (idx == 0) toolTimecalcQuick();
            else if (idx == 1) toolTimecalcCustom();
        } catch (UserCancelled&) {}
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  PROBCALC
// ═══════════════════════════════════════════════════════════════════════
void toolProbcalc() {
    screenSwipe("PROBCALC", THEME_YELLOW);
    FormPanel fp("DROP PROBABILITY", THEME_YELLOW,
                 "boxes needed to obtain a drop");

    double pct = fp.askNum("Chance per box (%)", "chance", 0, 100);
    double p   = pct / 100.0;

    if (p <= 0.0) {
        fp.sep();
        fp.qa.push_back({ "result", "✖ impossible — chance is 0" });
        fp.draw(); pauseAny(); return;
    }

    long long qty = fp.askIntDefault("How many do you want? (blank = 1)", "quantity", 1, 1, 10000);

    double expected = (double)qty / p;

    double boxCost = 0;
    bool hasCost = fp.askNumOpt("Cost per box (blank = skip)", "box cost", boxCost, 0);

    fp.thick();
    fp.addGrad("ESTIMATED BOXES", fmtNum(expected));

    if (hasCost) {
        fp.addGrad("TOTAL COST", fmtNum(boxCost * expected));
    }
    fp.thick();

    fp.draw();
    pauseAny();
}

// ═══════════════════════════════════════════════════════════════════════
//  ABOUT
// ═══════════════════════════════════════════════════════════════════════
struct SocialLink { string url; string display; };

void toolAbout() {
    screenSwipe("ABOUT", THEME_GREEN);

    bool wasOn = g_raw.on;
    g_raw.enable();
    hideCursor();

    vector<SocialLink> socials = {
        { "https://www.youtube.com/@wtvsad", "youtube.com/@wtvsad" },
        { "https://x.com/wtvsad", "x.com/wtvsad" },
        { "https://www.artstation.com/wtvsad", "artstation.com/wtvsad" },
        { "https://github.com/wtvsad", "github.com/wtvsad" },
        { "https://patreon.com/wtvsad?utm_medium=unknown&utm_source=join_link&utm_campaign=creatorshare_creator&utm_content=copyLink",
          "patreon.com/wtvsad" },
    };

    RGB HACK_DIM   {  0, 150,  60 };
    RGB HACK_MED   {  0, 200,  90 };
    RGB HACK_BRIGHT{  0, 255, 120 };
    RGB HACK_WHITE {220, 255, 230 };
    RGB HACK_BG    {  0,  20,  10 };
    RGB HACK_ERR   {255,  80, 100 };

    const int INNER = 74;

    int sel = 0;
    auto pulseBase   = steady_clock::now();
    auto statusUntil = pulseBase;
    string statusMsg;
    RGB statusCol = HACK_BRIGHT;

    while (true) {
        auto now = steady_clock::now();
        auto el  = duration_cast<milliseconds>(now - pulseBase).count();
        double phase = (sin(el * 0.0045) + 1.0) * 0.5;
        RGB hl = lerp(HACK_MED, HACK_WHITE, phase);
        bool showStatus = now < statusUntil;
        bool blinkOn = (el % 1000) < 500;
        string cursor = blinkOn ? "▮" : " ";

        auto row = [&](const string& content) {
            int len = (int)visibleLen(content);
            int pad = INNER - len;
            if (pad < 0) pad = 0;
            cout << "\033[2K" << string(INDENT, ' ') << fg(HACK_MED) << "│" << RESET
                 << content << string((size_t)pad, ' ')
                 << fg(HACK_MED) << "│" << RESET << "\n";
        };
        auto blank = [&]() { row(""); };

        auto borderTop = [&](const string& leftLabel) {
            int used = 1 + 1 + (int)visibleLen(leftLabel) + 1;
            int dash = (INNER + 2) - used - 1;
            if (dash < 0) dash = 0;
            cout << "\033[2K" << string(INDENT, ' ')
                 << fg(HACK_MED) << "┌─["
                 << fg(HACK_BRIGHT) << leftLabel
                 << fg(HACK_MED) << "]"
                 << repeatStr("─", dash) << "┐" << RESET << "\n";
        };

        auto borderBottom = [&](const string& info) {
            int used = 1 + 1 + (int)visibleLen(info) + 1;
            int dash = (INNER + 2) - used - 1;
            if (dash < 0) dash = 0;
            cout << "\033[2K" << string(INDENT, ' ')
                 << fg(HACK_MED) << "└─["
                 << fg(HACK_DIM) << info
                 << fg(HACK_MED) << "]"
                 << repeatStr("─", dash) << "┘" << RESET << "\n";
        };

        auto section = [&](const string& name) {
            int used = 1 + 1 + (int)visibleLen(name);
            int dash = (INNER + 2) - used - 1;
            if (dash < 0) dash = 0;
            cout << "\033[2K" << string(INDENT, ' ')
                 << fg(HACK_MED) << "├─["
                 << fg(HACK_BRIGHT) << name
                 << fg(HACK_MED) << "]"
                 << repeatStr("─", dash) << "┤" << RESET << "\n";
        };

        homeCursor();
        cout << "\033[2K\n\n\n\n";

        borderTop("wtvsad@amtool · v2.14");

        blank();
        row("  " + fg(HACK_BRIGHT) + BOLD + "$ " + RESET + fg(HACK_MED) + "cat README.md" + RESET);
        blank();
        row("  " + fg(HACK_WHITE) + "AMTOOL is a terminal calculator for the Adopt Me!" + RESET);
        row("  " + fg(HACK_WHITE) + "trading community. Cost out pet stacks, estimate" + RESET);
        row("  " + fg(HACK_WHITE) + "grind time, and work out box-drop probability —" + RESET);
        row("  " + fg(HACK_WHITE) + "all without leaving your terminal." + RESET);

        section("whoami");
        blank();
        row("  " + fg(HACK_BRIGHT) + BOLD + "$ " + RESET + fg(HACK_MED) + "whoami" + RESET);
        blank();
        row("  " + fg(HACK_WHITE) + BOLD + "@wtvsad" + RESET + "  "
            + fg(HACK_DIM) + "· creator of amtool" + RESET);
        row("  " + fg(HACK_WHITE) + "Uploading things I make." + RESET);
        blank();
        row("  " + fg(HACK_MED) + "CSE undergrad  ·  Cybersecurity Club member" + RESET);
        blank();
        row("  " + fg(HACK_MED) + "Topics I work in:" + RESET);
        row("  " + fg(HACK_DIM) + "  · Digital Art              · 3D Modelling" + RESET);
        row("  " + fg(HACK_DIM) + "  · Particle Simulation      · 2D Animation (vector)" + RESET);
        row("  " + fg(HACK_DIM) + "  · 3D Animation             · Game Development" + RESET);
        row("  " + fg(HACK_DIM) + "  · Video Edit               · Sound Design" + RESET);
        row("  " + fg(HACK_DIM) + "  · Programming" + RESET);
        blank();
        row("  " + fg(HACK_MED) + "\"Maybe SAD stands for SomeAmazingDude, who knows.\"" + RESET);
        row("  " + fg(HACK_DIM) + DIM + "(actually, first 3 letters of my legal last name)" + RESET);

        section("socials");
        blank();
        row("  " + fg(HACK_BRIGHT) + BOLD + "$ " + RESET + fg(HACK_MED) + "ls ~/socials/" + RESET);

        for (int i = 0; i < (int)socials.size(); ++i) {
            if (i == sel) {
                string plain = "  ▮ " + socials[i].display;
                int len = (int)visibleLen(plain);
                int pad = INNER - len;
                if (pad < 0) pad = 0;
                cout << "\033[2K" << string(INDENT, ' ') << fg(HACK_MED) << "│" << RESET
                     << bg(hl) << fg(HACK_BG) << BOLD
                     << plain << string((size_t)pad, ' ') << RESET
                     << fg(HACK_MED) << "│" << RESET << "\n";
            } else {
                row("    " + fg(HACK_MED) + socials[i].display + RESET);
            }
        }

        blank();
        row("  " + fg(HACK_BRIGHT) + BOLD + "$ " + RESET + fg(hl) + cursor + RESET);
        blank();

        borderBottom("↑↓ navigate · ⏎ open · esc back · " + currentClock());

        if (showStatus) {
            cout << "\033[2K" << string(INDENT, ' ') << fg(statusCol) << statusMsg << RESET << "\n";
        }
        cout << "\033[J" << flush;

        if (!keyAvailable(45)) continue;
        int k = readKey();
        if (k == 'U') {
            sel = (sel - 1 + (int)socials.size()) % (int)socials.size();
            statusUntil = pulseBase;
        }
        else if (k == 'D') {
            sel = (sel + 1) % (int)socials.size();
            statusUntil = pulseBase;
        }
        else if (k == '\n' || k == '\r') {
            bool ok = openUrl(socials[sel].url);
            statusMsg = ok ? ("✓ opened " + socials[sel].display)
                           : ("✖ failed " + socials[sel].display);
            statusCol = ok ? HACK_BRIGHT : HACK_ERR;
            statusUntil = steady_clock::now() + milliseconds(1500);
        }
        else if (k == 27 || k == 'q') break;
    }

    hideCursor();
    if (!wasOn) g_raw.disable();
}

// ═══════════════════════════════════════════════════════════════════════
//  Main menu
// ═══════════════════════════════════════════════════════════════════════
int runMainMenu() {
    vector<MenuEntry> items = {
        { "a", "Cost calculator",  "work out how much currency you need",    'a' },
        { "b", "Time calculator",  "how long until you reach your goal",     'b' },
        { "c", "Drop probability", "boxes required for a drop at % chance",  'c' },
        { "i", "About / credits",  "who made this thing",                    'i' },
    };
    return runSelectMenu("A M T O O L", items, THEME_BRAND,
                         "calculator suite", "esc quit");
}

// ═══════════════════════════════════════════════════════════════════════
//  main
// ═══════════════════════════════════════════════════════════════════════
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m; GetConsoleMode(hOut, &m);
    SetConsoleMode(hOut, m | 0x0004);
    SetConsoleCtrlHandler(ctrlHandler, TRUE);
#else
    signal(SIGINT, signalHandler);
#endif
    ios_base::sync_with_stdio(false);

    bootSequence();

    while (true) {
        screenSwipe("MAIN MENU", THEME_BRAND);
        int choice = runMainMenu();
        if (choice < 0) break;

        try {
            if      (choice == 0) toolNmcostMenu();
            else if (choice == 1) toolTimecalc();
            else if (choice == 2) toolProbcalc();
            else if (choice == 3) toolAbout();
        } catch (UserCancelled&) {}
    }

    clearScreen();
    g_raw.disable();
    showCursor();
    cout << "\n\n";
    for (auto& line : LOGO) {
        cout << string(INDENT, ' ') << gradientLine(line, PINK_C, CYAN_C) << "\n";
        this_thread::sleep_for(milliseconds(40));
    }
    cout << "\n" << string(INDENT, ' ') << fg(GRAY_C) << DIM;
    typewrite("thanks for using amtool — by @wtvsad", 14);
    cout << RESET << "\n\n";

    cout << string(INDENT, ' ') << fg(PURPLE_C) << BOLD << "find me online" << RESET << "\n\n";
    SocialLink socials[] = {
        { "https://www.youtube.com/@wtvsad", "youtube.com/@wtvsad" },
        { "https://x.com/wtvsad",            "x.com/wtvsad" },
        { "https://www.artstation.com/wtvsad","artstation.com/wtvsad" },
        { "https://github.com/wtvsad",       "github.com/wtvsad" },
        { "https://patreon.com/wtvsad",      "patreon.com/wtvsad" },
    };
    for (auto& s : socials) {
        cout << string(INDENT, ' ') << fg(CYAN_C) << osc8(s.url, s.display) << RESET << "\n";
        this_thread::sleep_for(milliseconds(60));
    }
    cout << "\n";
    this_thread::sleep_for(milliseconds(500));
    return 0;
}
