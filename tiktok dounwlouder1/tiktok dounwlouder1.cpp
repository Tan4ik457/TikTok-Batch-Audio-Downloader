#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <regex>      
#include <set>        
#define NOMINMAX
#include <Windows.h>

namespace fs = std::filesystem;

std::mutex queue_mutex;
std::mutex io_mutex;

std::atomic<int> success_count(0);
std::atomic<int> fail_count(0);
std::atomic<int> processed_count(0);

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    fs::path current_dir = fs::current_path();
    fs::path links_file = current_dir / "links.txt";
    fs::path saves_dir = current_dir / "saves";

    if (!fs::exists(saves_dir)) {
        if (fs::create_directory(saves_dir)) {
            std::cout << "[+] Папка 'saves' успешно создана.\n";
        }
        else {
            std::cerr << "[-] Ошибка при создании папки 'saves'.\n";
            system("pause");
            return 1;
        }
    }

    std::ifstream file(links_file);
    if (!file.is_open()) {
        std::cerr << "[-] Файл links.txt не найден по пути: " << links_file.string() << "\n";
        std::ofstream create_file(links_file);
        std::cerr << "[!] Я создал пустой файл links.txt. Вставьте туда сообщения из Telegram и запустите снова.\n";
        system("pause");
        return 1;
    }

    std::string full_text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::regex url_regex(R"(https?://(?:www\.|vm\.|vt\.)?tiktok\.com/[A-Za-z0-9_./?=&%-]+)");
    auto words_begin = std::sregex_iterator(full_text.begin(), full_text.end(), url_regex);
    auto words_end = std::sregex_iterator();

    std::vector<std::string> links;
    std::set<std::string> unique_check; 

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::string found_url = i->str();
        if (unique_check.insert(found_url).second) {
            links.push_back(found_url);
        }
    }

    if (links.empty()) {
        std::cout << "[!] В файле links.txt не найдено ни одной ссылки на TikTok.\n";
        system("pause");
        return 0;
    }

    const unsigned int MAX_THREADS = 8;
    unsigned int num_threads = std::min(static_cast<unsigned int>(links.size()), MAX_THREADS);

    std::cout << "[+] Извлечено чистых ссылок: " << links.size() << "\n";
    std::cout << "[+] Запускаю загрузку в " << num_threads << " потоков (i7-7700HQ mode)...\n\n";

    auto start_time = std::chrono::steady_clock::now();
    size_t current_index = 0;
    std::vector<std::thread> workers;

    for (unsigned int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&]() {
            while (true) {
                std::string link;

                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    if (current_index >= links.size()) {
                        break;
                    }
                    link = links[current_index];
                    current_index++;
                }

                std::string command = "yt-dlp --quiet --no-warnings -x --audio-format mp3 -P \"" +
                    saves_dir.string() + "\" -o \"%(title)s [%(id)s].%(ext)s\" \"" + link + "\"";

                int result = std::system(command.c_str());
                int done = ++processed_count;

                {
                    std::lock_guard<std::mutex> lock(io_mutex);
                    if (result == 0) {
                        success_count++;
                        std::cout << "[" << done << "/" << links.size() << "] [+] Скачано: " << link << "\n";
                    }
                    else {
                        fail_count++;
                        std::cerr << "[" << done << "/" << links.size() << "] [-] Ошибка:  " << link << "\n";
                    }
                }
            }
            });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    std::cout << "\n========================================\n";
    std::cout << "[+] Готово за " << elapsed_sec << " сек.!\n";
    std::cout << "[+] Успешно: " << success_count << " | Ошибок: " << fail_count << "\n";
    std::cout << "========================================\n";

    ShellExecuteA(NULL, "open", saves_dir.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);

    system("pause");
    return 0;
}
