# TikTok Batch Audio Downloader

A fast, lightweight, multithreaded C++17 command-line tool for batch-extracting MP3 audio from TikTok links. Built to process messy text pastes (such as forwarded Telegram messages with timestamps and usernames) without ads, queues, or manual link formatting.

## Features
- **Multithreaded Engine:** Uses `std::thread` and `std::mutex` (up to 8 concurrent workers) to download and convert multiple tracks simultaneously.
- **Smart Regex Parser:** Automatically extracts clean TikTok URLs (`tiktok.com`, `://tiktok.com`, `://tiktok.com`) from raw copied text and removes duplicates (`std::set`).
- **Collision-Free Naming:** Saves tracks as `Title [VideoID].mp3` so identically named sounds never overwrite each other.
- **Windows Integration:** Full UTF-8 console support (`SetConsoleOutputCP(65001)`) and automatic opening of the `saves` output folder upon completion.

## Requirements
- Windows OS (C++17 compatible compiler)
- [yt-dlp](https://github.com) (`yt-dlp.exe` placed in the same directory or added to system `PATH`)
- [FFmpeg](https://ffmpeg.org) (`ffmpeg.exe` and `ffprobe.exe` placed in the same directory or added to system `PATH`)

## Usage
1. Place `yt-dlp.exe` and `ffmpeg.exe` next to the compiled executable.
2. Create a `links.txt` file in the same folder and paste any text containing TikTok links (no need to clean up dates, names, or line breaks).
3. Run the executable. All MP3 files will be saved to the `saves/` folder, which opens automatically when finished.
