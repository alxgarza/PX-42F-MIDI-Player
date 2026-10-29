/* MIDI Player for the PX-42F: plays the Standard MIDI Files in
 * /MIDI on the calculator's speaker, using the firmware's synthesizer
 * (px->midi_play(), API version 4). The list, a progress bar with the time,
 * and 16 boxes for the MIDI channels, lit while the first 3/4 of a note sounds on them.
 *
 * Keys: up / down choose a song, ENTER plays it, R/S plays or stops,
 * BACKSPACE stops, Sigma+ switches between repeating all the songs and
 * playing each once. ON goes back to the menu (the music stops). */
#include <stdio.h>

#include "px.h"

#define SONGS_DIR   "/MIDI"
#define MAX_SONGS   64
#define LIST_ROWS   4
#define ROW_H       12
#define LIST_TOP    14
#define FRAME_MS    30

static char songs[MAX_SONGS][PX_FILE_NAME_MAX + 1];
static int song_count;
static int selected, top;
static int playing = -1; /* the song's index, or -1 */
static bool repeat_all = true;
static const char *message = ""; /* why the last song didn't play */

/* The songs on the drive; with none, the folder is made for the user. */
static void load_songs(void) {
    song_count = px->list(SONGS_DIR, ".mid", songs, MAX_SONGS);
    if (song_count == 0) {
        px->mkdir(SONGS_DIR);
    }
}

static void stop(void) {
    px->sound_stop();
    playing = -1;
}

static void start(int index) {
    char path[PX_PATH_MAX];
    snprintf(path, sizeof(path), SONGS_DIR "/%s.mid", songs[index]);
    int r = px->midi_play(path);
    playing = r == 0 ? index : -1;
    switch (r) {
        case 0: message = ""; break;
        case PX_MIDI_NOT_FOUND: message = "Can't read the file"; break;
        case PX_MIDI_NOT_MIDI: message = "Not a MIDI file it can play"; break;
        case PX_MIDI_TOO_BIG: message = "Too big (over 128 KB)"; break;
        default: message = "Not enough memory"; break;
    }
}

static void move(int by) {
    if (song_count == 0)
        return;
    selected = (selected + by + song_count) % song_count;
    if (selected < top)
        top = selected;
    if (selected >= top + LIST_ROWS)
        top = selected - LIST_ROWS + 1;
}

static void time_text(char *buf, size_t size, uint32_t ms) {
    snprintf(buf, size, "%lu:%02lu", (unsigned long) (ms / 60000), (unsigned long) (ms / 1000 % 60));
}

static void draw(void) {
    char line[48];
    px_midi_status_t st;
    px->midi_status(&st);

    px->clear();
    px->fill(0, 0, PX_SCREEN_W, 12, 1);
    px->text(2, 0, "MIDI Player", PX_FONT_SMALL, 1);
    snprintf(line, sizeof(line), "%s  %d/%d", repeat_all ? "Repeat" : "Once", song_count ? selected + 1 : 0, song_count);
    px->text(PX_SCREEN_W - 2 - px->text_width(line, PX_FONT_SMALL), 0, line, PX_FONT_SMALL, 1);

    if (song_count == 0) {
        px->text(4, LIST_TOP + 2, "No songs. Copy .mid files to", PX_FONT_SMALL, 0);
        px->text(4, LIST_TOP + 14, SONGS_DIR " on the drive", PX_FONT_SMALL, 0);
    }
    for (int row = 0; row < LIST_ROWS && top + row < song_count; row++) {
        int i = top + row, y = LIST_TOP + row * ROW_H;
        bool sel = i == selected;
        if (sel)
            px->fill(0, y, PX_SCREEN_W, ROW_H, 1);
        if (i == playing)
            px->text(2, y, "\006", PX_FONT_SMALL, sel); /* the play arrow */
        px->text(14, y, songs[i], PX_FONT_SMALL, sel);
    }
    px->line(0, 65, PX_SCREEN_W - 1, 65, 1);

    if (playing >= 0)
        px->text(5, 69, songs[playing], PX_FONT_SMALL, 0);
    else
        px->text(5, 69, message[0] ? message : "ENTER: play   R/S: stop", PX_FONT_SMALL, 0);

    /* The progress bar. */
    px->fill(5, 84, 240, 8, 1);
    px->fill(6, 85, 238, 6, 0);
    if (st.length_ms > 0) {
        uint32_t done = st.elapsed_ms < st.length_ms ? st.elapsed_ms : st.length_ms;
        px->fill(6, 85, (int) ((uint64_t) 238 * done / st.length_ms), 6, 1);
        char now[12], total[12];
        time_text(now, sizeof(now), st.elapsed_ms);
        time_text(total, sizeof(total), st.length_ms);
        snprintf(line, sizeof(line), "%s / %s", now, total);
        px->text(5, 94, line, PX_FONT_SMALL, 0);
        snprintf(line, sizeof(line), "%lu notes", (unsigned long) st.voices);
        px->text(PX_SCREEN_W - 5 - px->text_width(line, PX_FONT_SMALL), 94, line, PX_FONT_SMALL, 0);
    } else if (playing < 0) {
        px->text(5, 94, "\005: repeat or once", PX_FONT_SMALL, 0);
    }

    /* The channels: a box each, filled during the first 3/4 of each note (the tenth
     * is the drums). */
    for (int ch = 0; ch < 16; ch++) {
        int x = 5 + ch * 15;
        px->fill(x, 108, 12, 10, 1);
        if (!(st.channels >> ch & 1))
            px->fill(x + 1, 109, 10, 8, 0);
    }
    px->show();
}

int app_main(void) {
    load_songs();
    for (;;) {
        draw();
        int key = px->key(playing >= 0 ? FRAME_MS : -1);
        if (key == PX_KEY_QUIT)
            return PX_EXIT_MENU; /* the firmware stops the song */
        if (playing >= 0 && !px->sound_playing()) { /* it ended */
            int next = (playing + 1) % song_count;
            if (repeat_all)
                start(next), selected = next, move(0);
            else
                playing = -1;
        }
        if (key == PX_KEY_NONE || (key & PX_KEY_RELEASED))
            continue;
        switch (key) {
            case PX_KEY_ARROW_UP: move(-1); break;
            case PX_KEY_ARROW_DN: move(1); break;
            case PX_KEY_ENTER:
                if (song_count)
                    start(selected);
                break;
            case PX_KEY_RS:
                if (playing >= 0)
                    stop();
                else if (song_count)
                    start(selected);
                break;
            case PX_KEY_BACKSPACE: stop(); message = ""; break;
            case PX_KEY_SIGMA: repeat_all = !repeat_all; break;
        }
    }
}
