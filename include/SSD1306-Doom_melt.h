#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

// ---- Конфігурація дисплею ----
// Змінити розміри під свій дисплей при потребі
#define WIDTH 128
#define HEIGHT 64
#define BYTES_PER_COL (HEIGHT / 8)  // 8

// Вказівник на дисплей
static U8G2* doom_u8g2 = nullptr;

// Зовнішній інтерфейс
inline void doom_melt_set_display(U8G2* display) {
    doom_u8g2 = display;
}

// Масиви для melt-ефекту (будуть алокуватися динамічно)
static uint8_t* melt_delay = nullptr;
static uint8_t* melt_order = nullptr;

// ---- Функції для розбору XBM в колонки ----
inline void split_xbm_into_columns(const uint8_t* xbm, uint8_t* columns, int width, int height) {
    for (int x = 0; x < width; x++) {
        for (int y_byte = 0; y_byte < height / 8; y_byte++) {
            uint8_t value = 0;
            for (int bit = 0; bit < 8; bit++) {
                int y = y_byte * 8 + bit;
                if (y >= height) break;

                int byte_index = (y * ((width + 7) / 8)) + (x / 8);
                uint8_t byte = xbm[byte_index];

                if (byte & (1 << (x % 8))) {
                    value |= (1 << bit);
                }
            }
            columns[x * (height / 8) + y_byte] = value;
        }
    }
}

// ---- Функції для малювання ----
inline void draw_byte_pixels(uint8_t byte, int x, int y) {
    if (!doom_u8g2) return;
    for (int bit = 0; bit < 8; bit++) {
        if (byte & (1 << bit)) {
            doom_u8g2->drawPixel(x, y + bit);
        }
    }
}

inline void draw_column(int x, uint8_t* column) {
    for (int byte = 0; byte < BYTES_PER_COL; byte++) {
        draw_byte_pixels(column[byte], x, byte * 8);
    }
}

inline void draw_columns(uint8_t* columns) {
    for (int x = 0; x < WIDTH; x++) {
        draw_column(x, columns + x * BYTES_PER_COL);
    }
}

// ---- Копіювання колонок ----
inline void copy_column(uint8_t* dest, const uint8_t* src, int x) {
    for (int byte = 0; byte < BYTES_PER_COL; byte++) {
        dest[x * BYTES_PER_COL + byte] = src[x * BYTES_PER_COL + byte];
    }
}

inline void copy_columns(uint8_t* dest, const uint8_t* src) {
    for (int x = 0; x < WIDTH; x++) copy_column(dest, src, x);
}

// ---- Генерація затримок для melt-ефекту ----
inline void generate_doom_style_delays() {
    if (!melt_delay) return;

    const uint8_t max_delay = 15;
    const uint8_t group_size = 4;

    for (int x = 0; x < WIDTH; x += group_size) {
        uint8_t delay = random(0, max_delay + 1);
        for (int i = 0; i < group_size && x + i < WIDTH; i++) {
            melt_delay[x + i] = delay;
        }
    }

    // Перемішування затримок для хаотичності
    for (int i = 0; i < 10; i++) {
        int a = random(0, WIDTH);
        int b = random(0, WIDTH);
        uint8_t t = melt_delay[a];
        melt_delay[a] = melt_delay[b];
        melt_delay[b] = t;
    }
}

// ---- Melt ефект для однієї колонки ----
// one_color - якщо true, то просто "затирає" колір (без перевірки нового малюнка)
inline bool melt_column_bit(uint8_t* melt_column, const uint8_t* new_column, uint8_t local_melt_step, bool one_color = false) {
    if (local_melt_step >= HEIGHT) return true;

    uint8_t pixel_index = HEIGHT - 1 - local_melt_step; 
    uint8_t new_bit;
    if (!one_color) {
        new_bit = (new_column[pixel_index / 8] >> (pixel_index % 8)) & 0x01;
    } else {
        new_bit = 0; // якщо one_color, то завжди 0 (чорний)
    }

    // Зсув усіх байтів колонки вниз на 1 біт + додавання нового біта
    for (int8_t byte = BYTES_PER_COL - 1; byte >= 0; --byte) {
        uint8_t next_bit = (byte > 0) ? ((melt_column[byte - 1] & 0x80) >> 7) : new_bit;
        melt_column[byte] = (melt_column[byte] << 1) | next_bit;
    }

    // Перевірка чи колонка дорівнює новій колонці
    for (uint8_t n = 0; n < BYTES_PER_COL; ++n) {
        if (melt_column[n] != new_column[n]) return false;
    }
    return true;
}

// ---- Melt ефект для всіх колонок ----
inline bool melt_columns(uint8_t* melt_columns, const uint8_t* new_columns, uint8_t step, bool one_color = false) {
    bool all_melt_finished = true;
    for (uint8_t x = 0; x < WIDTH; x++) {
        if (step >= melt_delay[x]) {
            uint8_t local_step = step - melt_delay[x];
            uint8_t* calculated_melt_column = melt_columns + x * BYTES_PER_COL;
            const uint8_t* calculated_new_column = new_columns + x * BYTES_PER_COL;

            bool column_melt_finished = melt_column_bit(calculated_melt_column, calculated_new_column, local_step, one_color);
            if (!column_melt_finished) all_melt_finished = false;
        } else {
            all_melt_finished = false;
        }
    }
    return all_melt_finished;
}

// ---- Основна функція для плавної зміни кадру з melt ефектом ----
inline void doom_melt_frame_change(const uint8_t* old_image, const uint8_t* new_image) {
    if (!doom_u8g2) return;

    // Виділення пам'яті
    uint8_t* old_image_columns = (uint8_t*)malloc(WIDTH * BYTES_PER_COL);
    uint8_t* new_image_columns = (uint8_t*)malloc(WIDTH * BYTES_PER_COL);
    uint8_t* melt_columns_buf = (uint8_t*)malloc(WIDTH * BYTES_PER_COL);
    melt_delay = (uint8_t*)malloc(WIDTH);
    melt_order = (uint8_t*)malloc(WIDTH);

    if (!old_image_columns || !new_image_columns || !melt_columns_buf || !melt_delay || !melt_order) {
        free(old_image_columns);
        free(new_image_columns);
        free(melt_columns_buf);
        free(melt_delay);
        free(melt_order);
        return;
    }

    split_xbm_into_columns(new_image, new_image_columns, WIDTH, HEIGHT);
    split_xbm_into_columns(old_image, old_image_columns, WIDTH, HEIGHT);
    copy_columns(melt_columns_buf, old_image_columns);

    generate_doom_style_delays();

    uint8_t step = 0;
    bool melt_finished = false;

    while (!melt_finished) {
#if defined(ESP8266) || defined(ESP32)
        ESP.wdtDisable(); // Відключаємо вайтдога, щоб не висів у довгому циклі
#endif
        melt_finished = melt_columns(melt_columns_buf, new_image_columns, step);
        doom_u8g2->clearBuffer();
        draw_columns(melt_columns_buf);
        doom_u8g2->sendBuffer();

        if (melt_finished)
            delay(100);

#if defined(ESP8266) || defined(ESP32)
        ESP.wdtEnable(WDTO_8S);
#endif
        step++;
    }

    free(old_image_columns);
    free(new_image_columns);
    free(melt_columns_buf);
    free(melt_delay);
    free(melt_order);

    melt_delay = nullptr;
    melt_order = nullptr;
}

