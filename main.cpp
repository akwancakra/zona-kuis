/*
TASKS:
1. Tampilkan map number saat mengisi soal agar tau nomor soal yang sedang diisi
2. Buat menu sebelum memainkan kuis, seperti:
    - Mulai kuis -> memilih file
    - Buat file baru -> membuat file baru
    - Keluar
3. Buat menu untuk membuat soal, seperti:
    - Tambah soal -> menambah soal ke file
    - Edit soal -> mengedit soal yang sudah ada (hanya 1 soal)
    - Hapus soal -> menghapus soal dari file (hanya 1 soal)
    - Lihat soal -> menampilkan soal yang ada di file
    - Edit file -> mengedit semua soal(jika bisa)
    - Hapus file -> menghapus file
    - Simpan -> menyimpan perubahan ke file
    - Batal -> kembali ke menu sebelumnya
    - Keluar
*/
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>
#include <iomanip> // Untuk menggunakan setw()
#include <cstring>
#include "additional.h"

using namespace std;
namespace fs = filesystem;

// fungsi untuk menghapus semua isi layar
#ifdef _WIN32
#define CLEAR_SCREEN "cls" // windows
#else
#define CLEAR_SCREEN "clear" // linux
#endif

// VARIABEL YANG DIGUNAKAN UNTUK MEMBUAT GRAF DAN LAINNYA
const int MAX_TITLE_LENGTH = 50;
const int MAX_CATEGORY_LENGTH = 50;
const int MAX_ADJACENCYLIST_LENGTH = 50;
const int MAX_HISTORYQUIZ_LENGTH = 50;
const int MAX_SCORE_LENGTH = 50;

// VARIABEL UNTUK DATA USER
int restartapp;
int loggedRole;
string username;
string nama;

struct User
{
    string username;    // username
    string pw;          // password
    string cfpw;        // confirm password
    string namalengkap; // display nama
    string role;        // "1" Guru "2" Siswa
};
User user;

struct BarisDataUser
{
    int barispw = 1;
    int barisnama = 2;
    int barisrole = 3;
};
BarisDataUser barisDataUser;

struct Pertanyaan
{
    string teksPertanyaan;
    string pilihanA;
    string pilihanB;
    string pilihanC;
    string pilihanD;
    char jawabanBenar;
    char jawabanPengguna;
    Pertanyaan *prev; // pointer ke pertanyaan sebelumnya
    Pertanyaan *next; // pointer ke pertanyaan selanjutnya
};

struct Tampilan
{
    int jumlahPembatas;
};
Tampilan tampilan;

struct Riwayat
{
    string judul;
    string kategori;
    string nama;
    string skor;
};
Riwayat riwayat;

const int JUMLAH_MAX_PERTANYAAN = 100;

Pertanyaan pertanyaan[JUMLAH_MAX_PERTANYAAN];
int jumlahPertanyaan = 0;

// Inisialisasi fungsi
void loadQuestionsFromFile(string namaFile);
bool tampilkanPertanyaan(Pertanyaan &p, int nomorPertanyaan, bool isUbahJawaban);
void getScore();
void mainkanKuis();
void banner(string title, string author, int tolerance);
string center(string input, int width);
void tampilan_pembatas(int size);
void caraBermain();
void kuisSelesai(int score, string namaFile);
void cekJawaban();
void praMainMenu();
void mainMenuGuru();
void mainMenuSiswa();
void buatAkun();
void masukAkun();
void mainkanKuis();
// FITUR BARU
void riwayatPengerjaan();
void exportSoalKeCSV();

// FUNGSI TAMBAHAN
// Membuat Banner/title
void banner(string title, string author = "", int tolerance = 0)
{
    // Tolerance adalah varibale untuk menambahkan space size dalam banner, agar sesuai. Nilai Tolerance tidak boleh negatif
    if (tolerance < 0)
    {
        tolerance = 0;
    }
    int longestLength;
    // longest length
    if (title.length() > author.length())
    {
        longestLength = title.length() + tolerance;
    }
    else
    {
        longestLength = author.length() + tolerance;
    }

    // list of char
    char sym1 = '+'; // ┌
    char sym2 = '-'; // ─
    char sym3 = '+'; // ┐
    char sym4 = '|'; // │
    char sym5 = '+'; // └
    char sym6 = '+'; // ┘

    // make a border UI
    cout << sym1;
    for (int i = 0; i < (longestLength + 4); i++)
    {
        cout << sym2;
    }
    cout << sym3 << endl;

    cout << sym4 << " " << center(title, (longestLength + 3)) << "  " << string((longestLength - title.length()) / 2, ' ') + sym4 << endl;
    cout << sym4 << " " << center(author, (longestLength + 3)) << "  " << string((longestLength - author.length()) / 2, ' ') + sym4 << endl;
    cout << sym5;

    for (int i = 0; i < (longestLength + 4); i++)
    {
        cout << sym2;
    }
    cout << sym6 << endl;

    tampilan.jumlahPembatas = longestLength;

    return;
}

void tampilan_pembatas(int size = 0)
{
    char sym2 = '-'; // ─
    for (int i = 0; i < (size + 4); i++)
    {
        cout << sym2;
    }
    cout << endl;
}

string center(string input, int width = 113) // agar text menjadi ke tengah dari default lebar 113
{
    return string((width - input.length()) / 2, ' ') + input;
}

// QUIZ UNTUk MENAMPUNG DATA KUIS YANG AKAN DIGUNAKAN DI RIWAYAT KUIS
struct Quiz
{
    string title;
    string category;
    string score;

    Quiz(const string &t, const string &c, const string &s)
    {
        title = t;
        category = c;
        score = s;
    }
};

// BIKIN NODE-NYA
struct Node
{
    string name;
    Quiz *history[MAX_HISTORYQUIZ_LENGTH];
    int quizCount;

    Node(const string &n)
    {
        name = n;
        quizCount = 0;
    }

    void addQuiz(Quiz *quiz)
    {
        if (quizCount < MAX_HISTORYQUIZ_LENGTH)
        {
            history[quizCount++] = quiz;
        }
    }
};

// GRAPH UNTUK RIWAYAT
struct Graph
{
    int numVertices;
    Node *adjacencyList[MAX_ADJACENCYLIST_LENGTH];
    string titles[MAX_TITLE_LENGTH];
    string categories[MAX_CATEGORY_LENGTH];

    Graph(int vertices)
    {
        numVertices = vertices;
        for (int i = 0; i < numVertices; i++)
        {
            adjacencyList[i] = nullptr;
        }
    }

    void saveUserData()
    {
        ofstream file("./riwayat/data-user.csv");

        if (file.is_open())
        {
            for (int i = 0; i < numVertices; i++)
            {
                if (adjacencyList[i] != nullptr)
                {
                    string personName = adjacencyList[i]->name;
                    file << personName << endl;
                }
            }

            file.close();
            cout << "Data pengguna berhasil disimpan ke file: data-user.csv" << endl;
        }
        else
        {
            cout << "Gagal membuka file: data-user.csv" << endl;
        }
    }

    void saveQuizData()
    {
        ofstream file("./riwayat/data-kuis.csv");

        if (file.is_open())
        {
            for (int i = 0; i < MAX_CATEGORY_LENGTH; i++)
            {
                if (!titles[i].empty())
                {
                    file << titles[i] << ",";
                    file << categories[i] << endl;
                }
            }

            // file << title << ",";
            // file << category << endl;

            file.close();
            cout << "Data kuis berhasil disimpan ke file: data-kuis.csv" << endl;
        }
        else
        {
            cout << "Gagal membuka file: data-kuis.csv" << endl;
        }
    }

    void saveQuizHistoryData()
    {
        ofstream file("./riwayat/riwayat.csv");

        if (file.is_open())
        {
            for (int i = 0; i < numVertices; i++)
            {
                if (adjacencyList[i] != nullptr)
                {
                    string personName = adjacencyList[i]->name;

                    for (int j = 0; j < adjacencyList[i]->quizCount; j++)
                    {
                        Quiz *quiz = adjacencyList[i]->history[j];
                        file << personName << ",";
                        file << quiz->title << ",";
                        file << quiz->score << endl;
                    }
                }
            }
            // file << personName << ",";
            // file << title << ",";
            // file << score << endl;

            file.close();
            cout << "Data riwayat kuis berhasil disimpan ke file: riwayat.csv" << endl;
        }
        else
        {
            cout << "Gagal membuka file: riwayat.csv" << endl;
        }
    }

    void addPersonFromFile()
    {
        ifstream file("./riwayat/data-user.csv");

        if (file.is_open())
        {
            string name;
            while (getline(file, name))
            {
                addPerson(name);
            }

            file.close();
            cout << "Data pengguna berhasil ditambahkan dari file: data-user.csv" << endl;
        }
        else
        {
            cout << "Gagal membuka file: data-user.csv" << endl;
        }
    }

    void addQuizFromFile()
    {
        ifstream file("./riwayat/data-kuis.csv");

        if (file.is_open())
        {
            string title, category;
            while (getline(file, title, ',') && getline(file, category))
            {
                addQuiz(title, category);
            }

            file.close();
            cout << "Data kuis berhasil ditambahkan dari file: data-kuis.csv" << endl;
        }
        else
        {
            cout << "Gagal membuka file: data-kuis.csv" << endl;
        }
    }

    void addQuizHistoryFromFile()
    {
        ifstream file("./riwayat/riwayat.csv");

        if (file.is_open())
        {
            string personName, quizTitle, score;
            while (getline(file, personName, ',') && getline(file, quizTitle, ',') && getline(file, score))
            {
                addQuizHistory(personName, quizTitle, score);
            }

            file.close();
            cout << "Data riwayat kuis berhasil ditambahkan dari file: riwayat.csv" << endl;
        }
        else
        {
            cout << "Gagal membuka file: riwayat.csv" << endl;
        }
    }

    void addPerson(const string &name)
    {
        for (int i = 0; i < numVertices; i++)
        {
            if (adjacencyList[i] == nullptr)
            {
                Node *node = new Node(name);
                adjacencyList[i] = node;
                break;
            }
        }
    }

    void addQuiz(const string &title, const string &category)
    {
        for (int i = 0; i < MAX_TITLE_LENGTH; i++)
        {
            if (titles[i].empty())
            {
                titles[i] = title;
                categories[i] = category;
                break;
            }
        }
    }

    void addQuizHistory(const string &personName, const string &quizTitle, const string &score)
    {
        int personIndex = -1;
        int quizIndex = -1;

        for (int i = 0; i < numVertices; i++)
        {
            if (adjacencyList[i] != nullptr && adjacencyList[i]->name == personName)
            {
                personIndex = i;
                break;
            }
        }

        for (int i = 0; i < MAX_TITLE_LENGTH; i++)
        {
            if (titles[i] == quizTitle)
            {
                quizIndex = i;
                break;
            }
        }

        if (personIndex != -1 && quizIndex != -1)
        {
            Quiz *quiz = new Quiz(titles[quizIndex], categories[quizIndex], score);

            // Memastikan indeks kuis tidak melebihi batas
            if (adjacencyList[personIndex]->quizCount < MAX_HISTORYQUIZ_LENGTH)
            {
                adjacencyList[personIndex]->addQuiz(quiz);
            }
            else
            {
                cout << "Riwayat kuis penuh untuk anda" << endl;
            }
        }
        else
        {
            cout << "Pengguna tidak ditemukan" << endl;
        }
    }

    void printHistory(Node *current)
    {
        cout << "----" << current->name << "----" << endl;
        for (int j = 0; j < current->quizCount; j++)
        {
            Quiz *quiz = current->history[j];
            cout << "Kuis: " << quiz->title << endl;
            cout << "Kategori: " << quiz->category << endl;
            cout << "Skor: " << quiz->score << endl;
            cout << endl;
        }
    }

    void printGraph(const string &name, int loggedRole)
    {
        bool hasHistory = false;

        for (int i = 0; i < numVertices; i++)
        {
            Node *current = adjacencyList[i];
            if (current != nullptr && loggedRole == 2 && current->name == name)
            {
                if (current->quizCount > 0)
                {
                    printHistory(current);
                    hasHistory = true;
                }
            }
            else if (current != nullptr && loggedRole == 1)
            {
                if (current->quizCount > 0)
                {
                    printHistory(current);
                    hasHistory = true;
                }
            }
        }

        if (!hasHistory)
        {
            cout << "Belum ada riwayat pengerjaan kuis" << endl;
        }
    }
};

// INISIALISASI GRAPH UNTUK NANTI
int numPeople = 50;
Graph graph(numPeople);

// FUNGSI AUTH FIXED
void masukAkun()
{
    string line;
    string inputpw;
    string pw;
    int role;
    int percobaan = 3;

    system(CLEAR_SCREEN);

    banner("MASUK AKUN", "Zona Kuis - Kelompok 3", 10);
    cout << " [X] Kembali" << endl;
    tampilan_pembatas(tampilan.jumlahPembatas);

    cout << " Masukkan username : ";
    cin >> username;

    if (username == "x" || username == "X")
    {
        username = "";
        restartapp = 0;
        // praMainMenu();
        cin.get();
        return;
    }

    ifstream file("./users/" + username + ".txt");

    // JIKA FILE DITEMUKAN MAKA LANJUTKAN
    if (file.is_open())
    {
        // Mengambil data password dari baris pertama
        if (getline(file, line))
        {
            pw = line;
        }

        // Mengambil data nama dari baris kedua
        if (getline(file, line))
        {
            nama = line;
        }

        // Mengambil data role dari baris ketiga
        if (getline(file, line))
        {
            role = stoi(line);
        }

        file.close();
    }

    // JIKA FILE TIDAK DITEMUKAN MAKA KEMBALIKAN
    else
    {
        cout << " username tidak ditemukan harap buat akun baru" << endl;
        cout << " Tekan tombol apapun untuk lanjut..." << endl;
        cin.get();
        restartapp = 0;
        cin.ignore();
        return;
    }

    do
    {
        system(CLEAR_SCREEN);
        banner("MASUK AKUN", "Zona Kuis - Kelompok 3", 10);

        cout << " Masukkan username : " << username << endl;
        cout << " Masukkan password : ";
        cin >> inputpw;

        if (inputpw == pw)
        {
            cout << " Selamat datang" << endl;
            break;
        }
        else if (inputpw != pw)
        {
            cout << " Password salah" << endl;
            cout << " Silahkan input kembali, anda masih memiliki " << percobaan - 1 << " kali percobaan" << endl;
            cout << " Tekan tombol apapun untuk lanjut..." << endl;
            cin.get();
        }

        percobaan--;
        if (percobaan == 0)
        {
            cout << " Percobaan habis, coba kembali lain waktu" << endl;
            restartapp = 0;
            cin.ignore();
            return;
        }
    } while (inputpw != pw && percobaan > 0);

    cin.ignore();
    cout << endl;
    cout << " Tekan tombol apapun untuk lanjut..." << endl;
    cin.get();

    if (role == 1)
    {
        loggedRole = 1;
        mainMenuGuru();
    }
    else if (role == 2)
    {
        loggedRole = 2;
        mainMenuSiswa();
    }
    else
    {
        masukAkun();
    }
}

void buatAkun()
{
    char konfirmasi;
    bool suksesmenyimpan;
    bool syaratinput = false;

    do
    {
        system(CLEAR_SCREEN);
        user.namalengkap = "\0";

        do
        {
            banner("BUAT AKUN", "Zona Kuis - Kelompok 3", 20);
            cout << " Masukkan [0] pada username untuk kembali" << endl;
            cout << endl;

            do
            {
                cout << " Masukkan username     : ";
                getline(cin, user.username);
                if (user.username.length() <= 8 && user.username != "0")
                {
                    cout << " Jumlah karakter yang diinputkan harus lebih dari 8 karakter" << endl;
                    cout << endl;
                }
            } while (user.username.length() <= 8 && user.username != "0");

            if (user.username != "0")
            {
                do
                {
                    cout << " Masukkan password     : ";
                    getline(cin, user.pw);
                    if (user.pw.length() <= 8)
                    {
                        cout << " Jumlah karakter yang diinputkan harus lebih dari 8 karakter...." << endl;
                        cout << endl;
                    }
                } while (user.pw.length() <= 8);

                cout << " Konfirmasi passwordmu : ";
                getline(cin, user.cfpw);
                tampilan_pembatas(tampilan.jumlahPembatas);

                if (user.username.find(' ') != string::npos && user.username.find(' ') != string::npos && user.username.find(' ') && string::npos)
                {
                    cout << " Hindari penggunaan spasi" << endl;
                }
                else
                {
                    syaratinput = true;
                }

                if (user.pw != user.cfpw)
                {
                    cout << " Pastikan konfirmasi password sama dengan password" << endl;
                }
            }
            else if (user.username == "0")
            {
                restartapp = 0;
                return;
            }

            cout << " Tekan tombol apapun untuk lanjut..." << endl;
            cin.get();
            system("CLS");
        } while (user.pw != user.cfpw || !syaratinput);

        system(CLEAR_SCREEN);

        banner("LENGKAPI PROFILMU", "Zona Kuis - Kelompok 3", 10);
        cout << " Masukkan nama lengkapmu  : ";
        getline(cin, user.namalengkap);

        bool validInput = false;
        do
        {
            cout << " Kamu sebagai apa  : " << endl;
            cout << " [1] Guru" << endl;
            cout << " [2] Siswa" << endl;
            cout << " Masukan 1 atau 2 " << endl;

            getline(cin, user.role);

            // Validasi panjang string
            if (user.role.length() != 1)
            {
                cout << "Input tidak valid. Mohon masukkan 1 atau 2." << endl;
                continue;
            }

            // Validasi nilai role
            if (user.role != "1" && user.role != "2")
            {
                cout << "Input tidak valid. Mohon masukkan 1 atau 2." << endl;
                continue;
            }

            validInput = true;
        } while (!validInput);

        tampilan_pembatas(tampilan.jumlahPembatas);

        int pos = 0;
        for (int i = 0; i < user.namalengkap.length(); i++)
        {
            if (user.namalengkap[i] == ' ')
            {
                user.namalengkap[pos] = toupper(user.namalengkap[pos]);
                pos = i + 1;
            }
        }
        user.namalengkap[pos] = toupper(user.namalengkap[pos]);

        cout << " Akun akan dibuat dengan" << endl;
        cout << " Nama lengkap : " << user.namalengkap << endl;
        cout << " Role : " << (user.role == "1" ? "Guru" : "Siswa") << endl;
        cout << endl;

        cout << " Apakah data yang diinputkan benar? (Y/N) : ";
        cin >> konfirmasi;
        konfirmasi = toupper(konfirmasi);

        if (konfirmasi == 'Y')
        {

            ofstream file("./users/" + user.username + ".txt");

            if (file.is_open())
            {
                suksesmenyimpan = true;
                file << user.pw << endl;
                file << user.namalengkap << endl;
                file << user.role << endl;
                file.close();

                // NAMBAH USER PADA GRAPH
                graph.addPerson(user.namalengkap);
                graph.saveUserData();

                cout << " Akun telah berhasil dibuat" << endl;
            }
            else
            {
                suksesmenyimpan = false;
                cout << " gagal membuka file" << endl;
                cout << " Tekan tombol apapun untuk lanjut..." << endl;
                cin.get();
            }
        }

        cin.ignore();
    } while (konfirmasi == 'N' || suksesmenyimpan == false);

    cout << " Tekan tombol apapun untuk lanjut..." << endl;
    cin.get();
    restartapp = 0;
    return;
}

void praMainMenu()
{
    int pilihan;
    // validasi input
    bool validInput = false;

    do
    {

        system(CLEAR_SCREEN);
        banner("KUIS BERBASIS DIGITAL", "Zona Kuis - Kelompok 3", 10);
        cout << " Silahkan pilih menu:" << endl;
        cout << " [1] Masuk akun" << endl;
        cout << " [2] Buat akun" << endl;
        cout << " [3] Keluar Aplikasi" << endl;

        tampilan_pembatas(tampilan.jumlahPembatas);

        // while (!validInput)
        // {
        cout << " Pilihan: ";
        string input;
        getline(cin, input);

        if (input.empty())
        {
            cout << " Pilihan tidak boleh kosong!" << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
            continue;
        }

        bool isNumeric = true;
        for (char c : input)
        {
            if (!isdigit(c))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric)
        {
            pilihan = stoi(input);
            if (pilihan >= 1 && pilihan <= 3)
            {
                validInput = true;
            }
            else
            {
                cout << " Pilihan tidak valid, silahkan pilih 1,2 atau 3!" << endl;
                cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
                cin.get();
            }
        }
        else
        {
            cout << " Pilihan tidak valid, silahkan pilih 1,2 atau 3!" << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
        }
        // }
    } while (!validInput);

    if (pilihan == 1)
    {
        masukAkun();
    }
    else if (pilihan == 2)
    {
        buatAkun();
    }
    else if (pilihan == 3)
    {
        system(CLEAR_SCREEN);
        banner("Terima Kasih Sudah Menggunakan Aplikasi Kami!", "Kelompok 3 - PAMIT", 0);
        cout << " App Developed By: " << endl;
        cout << " AKWAN CAKRA TAJIMALELA    - 2209098" << endl;
        cout << " KRISNA SANTOSA            - 2209092" << endl;
        cout << " RAFLY IVAN KHALFANI       - 2202187" << endl;
        cout << " RAMANDHA PUTRA SURYAHADI  - 2200125" << endl;
        cout << " ";
        tampilan_pembatas(tampilan.jumlahPembatas);
        cout << " Tekan tombol apapun untuk keluar..." << endl;
        cin.get();
        exit(0);
    }
    else
    {
        cout << " Pilihan tidak valid!" << endl;
        cout << " Tekan tombol apapun untuk kembali ke menu utama..." << endl;
        cin.get();
        cin.get();
        // clear the console
        system(CLEAR_SCREEN);
        praMainMenu();
    }
}

void caraBermain()
{
    system(CLEAR_SCREEN);
    banner("Cara Bermain ZonaKuis", "Kelompok 3", 5);
    cout << "1. Pilih menu 'Mulai Kuis' untuk memulai kuis" << endl;
    cout << "2. Pilih file pertanyaan yang akan digunakan" << endl;
    cout << "3. Setelah pertanyaan tampil, pilih jawaban yang menurut anda benar" << endl;
    cout << "4. Anda dapat menjawab dengan huruf 'A', 'B', 'C', atau 'D'" << endl;
    cout << "5. Jika anda ingin navigasi ke pertanyaan sebelumnya gunakan huruf 'P' " << endl;
    cout << "6. Jika anda ingin navigasi ke pertanyaan selanjutnya gunakan huruf 'N' " << endl;
    cout << "7. Setelah selesai mengisi semua pertanyaan, akan muncul menu yang menampilkan: " << endl;
    cout << "   - Tampilkan Jawaban Saya" << endl;
    cout << "   - Ubah Jawaban" << endl;
    cout << "   - Kirim Jawaban" << endl;
    cout << "8. Menu 'Ubah Jawaban' memudahkan anda untuk mengganti jawaban hanya dengan memasukan nomor pertanyaan saja" << endl;
    cout << "9. Menu 'Tampilkan Jawaban Saya' akan menampilkan jawaban yang telah anda isi" << endl;
    cout << "10. Menu 'Kirim Jawaban' akan mengirim jawaban yang telah anda isi, dan score akan langsung keluar" << endl;
    tampilan_pembatas(tampilan.jumlahPembatas);
}

// FUNGSI UTAMA
void riwayatPengerjaan()
{
    system(CLEAR_SCREEN);
    banner("Riwayat Pengerjaan", "Guru " + username, 5);

    // Menampilkan graf
    graph.printGraph(nama, loggedRole);

    cout << "Tekan enter untuk melanjutkan..." << endl;
    cin.get();

    // // Open the riwayat.txt file for reading
    // ifstream bacaRiwayat("./questions/riwayat.txt");

    // // Check if the file was opened successfully
    // if (!bacaRiwayat.is_open())
    // {
    //     cout << "Belum ada riwayat pengerjaan kuis." << endl;
    //     system("PAUSE");
    //     return;
    // }

    // // Read the contents of the file and print them to the console
    // string line;
    // while (getline(bacaRiwayat, line))
    // {
    //     cout << line << endl;
    // }

    // // Close the file
    // bacaRiwayat.close();
}

void exportSoalKeCSV()
{
    // path: questions/category/soal.csv
    system(CLEAR_SCREEN);
    string directoryPath = "./questions"; // path ke folder pertanyaan
    int pilihFolder;

    // jika nomor file tidak valid, minta user untuk memasukkan nomor file yang valid
    // kode supaya tidak ada shortcut
    bool validInputFolder = false;

    while (!validInputFolder)
    {
        system(CLEAR_SCREEN);

        banner("Export ke CSV", "ZONA KUIS - Kelompok 3", 10);
        // pilih kategori kuis dari folder questions
        cout << " Pilih kategori kuis:\n";

        int numfile = 1;
        bool folderFound = false; // Tambahkan variabel untuk menandai jika ada folder yang ditemukan

        for (const auto &entry : fs::directory_iterator(directoryPath))
        {
            if (entry.is_directory())
            {
                cout << " [" << numfile << "] " << entry.path().filename() << endl;
                numfile++;
                folderFound = true; // Set folderFound menjadi true jika ada folder yang ditemukan
            }
        }

        // Cek jika tidak ada folder yang ditemukan
        if (!folderFound)
        {
            cout << " [!] Belum ada folder [!]" << endl;
        }
        cout << " [X] Untuk kembali\n";

        cout << " Masukkan nomor kategori yang ingin di export: ";
        string input;
        getline(cin, input);

        // cek apakah kembali
        if (input == "x" || input == "X")
        {
            if (loggedRole == 1)
            {
                mainMenuGuru();
                return;
            }
            else
            {
                mainMenuSiswa();
                return;
            }
        }

        if (input.empty())
        {
            cout << " Pilihan tidak boleh kosong!" << endl;
            system("pause");
            continue;
        }

        bool isNumeric = true;
        for (char c : input)
        {
            if (!isdigit(c))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric)
        {
            pilihFolder = stoi(input);
            if (pilihFolder >= 1 && pilihFolder <= numfile - 1)
            {
                validInputFolder = true;
            }
            else
            {
                cout << "\n Nomor kategori tidak valid! Silahkan masukkan nomor kategori yang valid." << endl;
                cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
                cin.get();

                if (loggedRole == 2)
                {
                    mainkanKuis();
                    return;
                }
            }
        }
        else
        {
            cout << "\n Nomor kategori tidak valid! Silahkan masukkan nomor kategori yang valid." << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();

            if (loggedRole == 2)
            {
                mainkanKuis();
                return;
            }
        }
    }

    // masukan pilihan kategori folderPath
    string folderPath;
    int folderNum = 1;
    // list all folder
    for (const auto &entry : fs::directory_iterator(directoryPath))
    {
        if (entry.is_directory())
        {
            if (folderNum == pilihFolder)
            {
                folderPath = entry.path().string();
            }
            folderNum++;
        }
    }

    system(CLEAR_SCREEN);

    // Pilih file
    int pilihFile;
    string fullPath;

    // jika nomor file tidak valid, minta user untuk memasukkan nomor file yang valid
    // kode supaya tidak ada shortcut
    bool validInput = false;
    while (!validInput)
    {
        system(CLEAR_SCREEN);

        cout << " Pilih file kuis:\n";
        int fileNum = 1;
        for (const auto &entry : fs::directory_iterator(folderPath))
        {
            if (entry.is_regular_file())
            {
                cout << " [" << fileNum << "] " << entry.path().filename() << endl;
                fileNum++;
            }
        }
        cout << " [X] Untuk kembali\n";

        cout << endl;
        tampilan_pembatas(tampilan.jumlahPembatas);

        cout << " Masukkan nomor file yang ingin dimuat: ";
        string input;
        getline(cin, input);

        // cek apakah kembali
        if (input == "x" || input == "X")
        {
            if (loggedRole == 1)
            {
                exportSoalKeCSV();
                return;
            }
            else
            {
                exportSoalKeCSV();
                return;
            }
        }

        if (input.empty())
        {
            cout << " Pilihan tidak boleh kosong!" << endl;
            system("pause");
            continue;
        }

        bool isNumeric = true;
        for (char c : input)
        {
            if (!isdigit(c))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric)
        {
            pilihFile = stoi(input);
            if (pilihFile >= 1 && pilihFile <= fileNum - 1)
            {
                validInput = true;
            }
            else
            {
                cout << "\n Nomor file tidak valid! Silahkan masukkan nomor file yang valid." << endl;
                cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
                cin.get();

                if (loggedRole == 2)
                {
                    mainkanKuis();
                    return;
                }
            }
        }
        else
        {
            cout << "\n Nomor file tidak valid! Silahkan masukkan nomor file yang valid." << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();

            if (loggedRole == 2)
            {
                mainkanKuis();
                return;
            }
        }
    }

    // Load pertanyaan dari file yang dipilih
    int numFile = 1;
    for (const auto &entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            if (numFile == pilihFile)
            {
                fullPath = entry.path().string();
                break; // Stop searching after finding the desired file
            }
            numFile++;
        }
    }

    // minta nama file
    string namaFile;
    cout << " Masukkan nama file untuk export: ";
    getline(cin, namaFile);

    // simpan namaFile.csv ke fullpath
    string csvPath = "./questions/" + namaFile + ".csv";

    // Open the file for reading
    ifstream inputFile(fullPath);

    // Open the output file for writing
    ofstream outputFile(csvPath);

    // Write the CSV header
    outputFile << "Pertanyaan,PilihanA,PilihanB,PilihanC,PilihanD,Jawaban" << endl;

    // Read all the lines from the input file, first line is the question, next 4 lines are the choices, last line is the answer
    string line;
    while (getline(inputFile, line))
    {
        outputFile << line << ","; // Write the question to the CSV file
        for (int i = 0; i < 4; i++)
        {
            getline(inputFile, line);
            outputFile << line << ","; // Write the choice to the CSV file
        }
        getline(inputFile, line);
        outputFile << line << endl; // Write the answer to the CSV file
    }

    cout << "File CSV berhasil dibuat!" << endl;
    cout << "Tekan enter untuk melanjutkan..." << endl;
    cin.get();

    // if (loggedRole == 1)
    // {
    //     mainMenuGuru();
    //     return;
    // }
    // else
    // {
    //     mainMenuSiswa();
    //     return;
    // }
}

void cekJawaban()
{
    // Jika jawaban pengguna kosong, berikan warning
    int nomorPertanyaan = 1;
    for (int i = 0; i < jumlahPertanyaan; i++)
    {
        if (pertanyaan[i].jawabanPengguna == ' ')
        {
            cout << " No " << nomorPertanyaan << " Masih Kosong!" << endl;
        }
        nomorPertanyaan++;
    }
}

void kuisSelesai(int score = 0, string namaFile = "", string judulKuis = "")
{
    system(CLEAR_SCREEN);
    banner("KUIS SELESAI", "ZONA KUIS - Kelompok 3", 10);
    // cout << " [1] Tampilkan jawaban saya\n";

    for (int i = 0; i < jumlahPertanyaan; i++)
    {
        char jawaban = pertanyaan[i].jawabanPengguna;
        string jawabanStr = (jawaban == '\0' || jawaban == ' ') ? "Belum diisi" : string(1, jawaban);
        cout << " Pertanyaan " << i + 1 << ": " << jawabanStr << endl;
    }

    cout << endl;
    tampilan_pembatas(tampilan.jumlahPembatas);

    cout << " [1] Ubah Jawaban\n";
    cout << " [2] Kirim jawaban\n";
    cout << endl;
    tampilan_pembatas(tampilan.jumlahPembatas);

    int pilihan;
    bool menuValidInput = false;

    while (!menuValidInput)
    {
        cout << " Pilihan Anda: ";
        string menuInput;
        getline(cin, menuInput);

        if (menuInput.empty())
        {
            cout << " Pilihan tidak boleh kosong!" << endl;
            continue;
        }

        bool isNumeric = true;
        for (char c : menuInput)
        {
            if (!isdigit(c))
            {
                isNumeric = false;
                break;
            }
        }
        if (isNumeric)
        {
            pilihan = stoi(menuInput);
            if (pilihan >= 1 && pilihan <= 3)
            {
                menuValidInput = true;
            }
            else
            {
                cout << "\n Pilihan tidak valid! Silahkan masukkan pilihan yang valid." << endl;
            }
        }
        else
        {
            cout << " Input tidak valid! Silahkan masukkan pilihan yang valid." << endl;
        }
    }

    // if (pilihan == 1)
    // {
    // system(CLEAR_SCREEN);
    // banner("JAWABAN ANDA", "ZONA KUIS - Kelompok 3", 10);
    // // menampilkan semua jawaban user
    // for (int i = 0; i < jumlahPertanyaan; i++)
    // {
    //     cout << " Pertanyaan " << i + 1 << ": " << pertanyaan[i].jawabanPengguna << endl;
    // }
    // cout << endl;
    // tampilan_pembatas(tampilan.jumlahPembatas);
    // cout << " Tekan tombol apapun untuk kembali ke menu utama..." << endl;
    // cin.get();
    // system(CLEAR_SCREEN);
    // kuisSelesai(score);
    // }
    // else
    if (pilihan == 1)
    {
        // mengubah jawaban
        int nomorPertanyaan;
        bool isNomorPertanyaanValid = false;
        while (!isNomorPertanyaanValid)
        {
            cout << " Masukkan nomor pertanyaan yang ingin diubah: ";
            string nomorPertanyaanInput;
            getline(cin, nomorPertanyaanInput);

            if (nomorPertanyaanInput.empty())
            {
                cout << " Pilihan tidak boleh kosong!" << endl;
                continue;
            }

            bool isNumeric = true;
            for (char c : nomorPertanyaanInput)
            {
                if (!isdigit(c))
                {
                    isNumeric = false;
                    break;
                }
            }
            if (isNumeric)
            {
                nomorPertanyaan = stoi(nomorPertanyaanInput);
                if (nomorPertanyaan >= 1 && nomorPertanyaan <= jumlahPertanyaan)
                {
                    isNomorPertanyaanValid = true;
                }
                else
                {
                    cout << "\n Nomor pertanyaan tidak valid! Silahkan masukkan nomor pertanyaan yang valid." << endl;
                }
            }
            else
            {
                cout << " Input tidak valid! Silahkan masukkan nomor pertanyaan yang valid." << endl;
            }
        }
        tampilkanPertanyaan(pertanyaan[nomorPertanyaan - 1], nomorPertanyaan, true);
        kuisSelesai(score);
    }
    else if (pilihan == 2)
    {
        // cek apakah ada jawaban yang masih kosong
        cekJawaban();
        cout << " Apakah anda yakin ingin mengirim jawaban? (Y/N)" << endl;
        cout << " Pilihan: ";

        string pil;
        char pilihanKirim;

        bool invalidInput = false;
        do
        {
            cout << " Masukkan pilihan (Y/N): ";
            getline(cin, pil);
            if (pil.length() == 1 && (pil[0] == 'Y' || pil[0] == 'N' || pil[0] == 'y' || pil[0] == 'n'))
            {
                pilihanKirim = pil[0];
                invalidInput = true;
                break;
            }
            cout << " Input tidak valid. Mohon masukkan Y atau N." << endl;
        } while (!invalidInput);
        pilihanKirim = toupper(pilihanKirim);

        // while (pilihanKirim != 'Y' && pilihanKirim != 'N')
        // {
        //     cout << " Pilihan tidak valid! Silahkan masukkan Y atau N" << endl;
        //     cout << " Apakah ingin menambah pertanyaan? (Y/N): ";
        //     cin >> pilihanKirim;
        //     cin.ignore();
        //     pilihanKirim = toupper(pilihanKirim);
        // }

        if (pilihanKirim == 'N')
        {
            system(CLEAR_SCREEN);
            kuisSelesai(score);
        }

        // ofstream riwayatFile("./questions/riwayat.txt", std::ios::app);

        // int panjangJudul = judulKuis.length();
        // string strip = string(panjangJudul, '-');
        // string capitalizeJudul = capitalizeString(judulKuis);

        // riwayatFile << left << setw(judulKuis.length() + 2) << setfill('-') << "-" << capitalizeJudul << "-" << setfill('-') << setw(judulKuis.length() + 2) << "-" << endl;
        // riwayatFile << "Nama: " << nama << endl;
        int skor = 0;
        for (int i = 0; i < jumlahPertanyaan; i++)
        {
            if (pertanyaan[i].jawabanPengguna == pertanyaan[i].jawabanBenar)
            {
                skor++;
            }
        }

        // size_t lastBackslashPos = namaFile.find_last_of('\\');
        // string categoryName = namaFile.substr(lastBackslashPos + 1);
        string finalSkor = to_string(skor) + "/" + to_string(jumlahPertanyaan);

        // riwayatFile << "Kategori: " << categoryName << endl;
        // riwayatFile << "Skor: " << skor << "/" << jumlahPertanyaan << endl;
        // riwayatFile << left << setw(judulKuis.length() + 2) << setfill('-') << "-" << strip << "-" << setfill('-') << setw(judulKuis.length() + 2) << "-" << endl;
        // riwayatFile << endl;

        // mengirim jawaban
        system(CLEAR_SCREEN);
        // MENAMBAHKAN RIWAYAT KUIS PADA GRAPH

        graph.addQuizHistory(nama, judulKuis, finalSkor);
        graph.saveQuizHistoryData();

        cout << nama << " " << judulKuis << " " << finalSkor << endl;
        system("pause");
        getScore();

        // inisialisasi ulang jumlah pertanyaan
        jumlahPertanyaan = 0;

        // wait for user to press enter
        cout << " Tekan Enter untuk kembali ke menu utama...";
        cin.get();

        mainMenuSiswa();
    }
    else
    {
        cout << " Pilihan tidak valid, silahkan ulangi.\n";
        cin.clear();
    }
}

void loadQuestionsFromFile(string namaFile)
{
    fs::path pathFile = fs::path(namaFile);
    ifstream file(namaFile);
    string baris;
    Pertanyaan *prev = nullptr; // inisialisasi pointer prev dengan null
    while (getline(file, baris))
    {
        Pertanyaan *p = new Pertanyaan(); // wadah baru untuk pertanyaan

        // inisisalisasi pertanyaan
        p->teksPertanyaan = baris;
        for (int i = 0; i < 4 && getline(file, baris) && !baris.empty(); i++)
        {
            switch (i)
            {
            case 0:
                p->pilihanA = baris.substr(3);
                break;
            case 1:
                p->pilihanB = baris.substr(3);
                break;
            case 2:
                p->pilihanC = baris.substr(3);
                break;
            case 3:
                p->pilihanD = baris.substr(3);
                break;
            }
        }
        if (getline(file, baris) && !baris.empty())
        {
            p->jawabanBenar = baris.substr(baris.size() - 1)[0];
        }
        else
        {
            p->jawabanBenar = ' ';
        }
        p->jawabanPengguna = ' ';
        p->prev = prev;    // set pertanyaan sebelumnya
        p->next = nullptr; // set pertanyaan selanjutnya dengan null

        if (prev != nullptr)
        {
            prev->next = p; // set pertanyaan selanjutnya dari pertanyaan sebelumnya
        }
        prev = p; // update pertanyaan sebelumnya

        // tambahkan pertanyaan ke array pertanyaan
        pertanyaan[jumlahPertanyaan] = *p;
        jumlahPertanyaan++;
    }
}

bool tampilkanPertanyaan(Pertanyaan &p, int nomorPertanyaan, bool isUbahJawaban = false)
{
    string input;
    bool isValid = false;

    do
    {

        system(CLEAR_SCREEN);

        if (!isUbahJawaban)
        {
            cout << " Nomor " << endl;
            for (int i = 0; i < jumlahPertanyaan; i++)
            {
                if (nomorPertanyaan == i + 1)
                {
                    cout << " " << i + 1 << " | ";
                }
                else
                {
                    changeColor(8);
                    cout << " " << i + 1;
                    changeColor(7);
                    cout << " | ";
                }

                if (i == 5)
                {
                    cout << "\n ";
                }
            }
            cout << "\n\n";
        }

        cout << " Pertanyaan " << nomorPertanyaan << ": " << p.teksPertanyaan << endl;
        cout << " A. " << p.pilihanA << endl;
        cout << " B. " << p.pilihanB << endl;
        cout << " C. " << p.pilihanC << endl;
        cout << " D. " << p.pilihanD << endl;

        cout << endl;

        if (!isUbahJawaban)
        {
            cout << " Gunakan huruf 'P' untuk kembali ke pertanyaan sebelumnya" << endl;
            cout << " Gunakan huruf 'N' untuk lanjut ke pertanyaan berikutnya" << endl;
            cout << " Masukan nomor soal untuk cepat" << endl;
        }
        tampilan_pembatas(tampilan.jumlahPembatas);

        // while (!isValid)
        // {
        cout << " Jawaban Anda: ";
        getline(cin, input);
        // loop through each character of the string and convert to uppercase
        for (char &c : input)
        {
            c = std::toupper(c);
        }

        input = regex_replace(input, regex("[^a-zA-Z]+"), ""); // remove all non-alphabetic characters

        if (input.empty())
        {
            cout << " Jawaban tidak boleh kosong! Silahkan masukkan jawaban A, B, C, atau D" << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
        }
        else if ((input.length() > 1 || input[0] < 'A' || input[0] > 'D') && (input[0] != 'P' && input[0] != 'N'))

        {
            cout << " Jawaban tidak valid! Silahkan masukkan jawaban A, B, C, atau D" << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
        }
        else
        {
            p.jawabanPengguna = toupper(input[0]);
            isValid = true;
            if (isUbahJawaban)
            {
                cout << "Berhasil diubah!, tekan tombol apapun untuk melanjutkan" << endl;
                cin.ignore(1000, '\n'); // menghapus input yang tidak valid
                system(CLEAR_SCREEN);
                break;
            }
        }
        // }
    } while (!isValid);

    // cek apakah user ingin kembali ke pertanyaan sebelumnya
    if (p.jawabanPengguna == 'P')
    {
        p.jawabanPengguna = ' ';
        return true;
    }

    // cek apakah user ingin ke pertanyaan selanjutnya
    if (p.jawabanPengguna == 'N')
    {
        p.jawabanPengguna = ' ';
        return false;
    }

    return false;
}

void getScore()
{
    banner("HASIL KUIS ANDA", "Kelompok 3", 10);
    int score = 0;
    for (int i = 0; i < jumlahPertanyaan; i++)
    {
        if (pertanyaan[i].jawabanPengguna == pertanyaan[i].jawabanBenar)
        {
            score++;
        }
    }
    cout << " TERIMA KASIH TELAH MENGERJAKAN KUIS, BERIKUT HASILNYA" << endl;
    cout << " Score: " << score << "/" << jumlahPertanyaan << endl;
}

void mainkanKuis()
{
    system(CLEAR_SCREEN);
    // pilih kategori kuis dari folder questions
    string directoryPath = "./questions"; // path ke folder pertanyaan

    // jika nomor file tidak valid, minta user untuk memasukkan nomor file yang valid
    // kode supaya tidak ada shortcut
    bool validInputFolder = false;
    int pilihFolder;
    do
    {
        int numfile = 1;

        system(CLEAR_SCREEN);
        banner("MULAI KUIS", "ZONA KUIS - Kelompok 3", 10);
        cout << " Pilih kategori kuis:\n";
        for (const auto &entry : fs::directory_iterator(directoryPath))
        {
            if (entry.is_directory())
            {
                cout << " [" << numfile << "] " << entry.path().filename() << endl;
                numfile++;
            }
        }
        cout << " [X] Kembali\n";

        cout << " Masukkan nomor kategori yang ingin dimuat: ";
        string input;
        getline(cin, input);

        if (input == "x" || input == "X")
        {
            mainMenuSiswa();
            return;
        }

        if (input.empty())
        {
            cout << " Pilihan tidak boleh kosong!" << endl;
            continue;
        }

        bool isNumeric = true;
        for (char c : input)
        {
            if (!isdigit(c))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric)
        {
            pilihFolder = stoi(input);

            if (pilihFolder >= 1 && pilihFolder <= numfile - 1)
            {
                validInputFolder = true;
            }
            else
            {
                cout << "\n Nomor kategori tidak valid! Silahkan masukkan nomor kategori yang valid." << endl;
                cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
                cin.get();
                validInputFolder = false;
                continue;
                // mainkanKuis();
            }
        }
        else
        {
            cout << "\n Nomor kategori tidak valid! Silahkan masukkan nomor kategori yang valid." << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
            validInputFolder = false;
            continue;
            // mainkanKuis();
        }

        validInputFolder = true;
    } while (!validInputFolder);

    // masukan pilihan kategori folderPath
    string folderPath;
    int folderNum = 1;
    // list all folder
    for (const auto &entry : fs::directory_iterator(directoryPath))
    {
        if (entry.is_directory())
        {
            if (folderNum == pilihFolder)
            {
                folderPath = entry.path().string();
            }
            folderNum++;
        }
    }

    system(CLEAR_SCREEN);

    // jika nomor file tidak valid, minta user untuk memasukkan nomor file yang valid
    // kode supaya tidak ada shortcut
    bool validInput = false;
    int pilihFile;
    do
    {
        system(CLEAR_SCREEN);
        cout << " Pilih file kuis:\n";
        int fileNum = 1;
        for (const auto &entry : fs::directory_iterator(folderPath))
        {
            if (entry.is_regular_file())
            {
                cout << " [" << fileNum << "] " << entry.path().filename() << endl;
                fileNum++;
            }
        }
        cout << " [X] Kembali\n";

        cout << endl;
        tampilan_pembatas(tampilan.jumlahPembatas);

        // Pilih file

        cout << " Masukkan nomor file yang ingin dimuat: ";
        string input;
        getline(cin, input);

        if (input == "x" || input == "X")
        {
            mainkanKuis();
            return;
        }

        if (input.empty())
        {
            cout << " Pilihan tidak boleh kosong!" << endl;
            continue;
        }

        bool isNumeric = true;
        for (char c : input)
        {
            if (!isdigit(c))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric)
        {
            pilihFile = stoi(input);
            if (pilihFile >= 1 && pilihFile <= fileNum - 1)
            {
                validInput = true;
            }
            else
            {
                cout << "\n Nomor file tidak valid! Silahkan masukkan nomor file yang valid." << endl;
                cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
                cin.get();
                system(CLEAR_SCREEN);
                validInputFolder = false;
                continue;
                // mainkanKuis();
            }
        }
        else
        {
            cout << "\n Nomor file tidak valid! Silahkan masukkan nomor file yang valid." << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
            system(CLEAR_SCREEN);
            validInputFolder = false;
            continue;
            // mainkanKuis();
        }

        validInput = true;
    } while (!validInput);

    // Load pertanyaan dari file yang dipilih
    int fileNum = 1;
    string selectedFileName;
    for (const auto &entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            if (fileNum == pilihFile)
            {
                selectedFileName = entry.path().filename().string();
                loadQuestionsFromFile(entry.path().string());
            }
            fileNum++;
        }
    }

    int score = 0;
    int indexPertanyaanSaatIni = 0;

    while (indexPertanyaanSaatIni < jumlahPertanyaan)
    {
        bool kePertanyaanSelanjutnya = tampilkanPertanyaan(pertanyaan[indexPertanyaanSaatIni], indexPertanyaanSaatIni + 1);

        if (kePertanyaanSelanjutnya) // true or false
        {
            if (indexPertanyaanSaatIni > 0)
            {
                indexPertanyaanSaatIni--;
            }
            else
            {
                cout << " Anda sudah berada di pertanyaan pertama. Tidak dapat pindah ke pertanyaan sebelumnya." << endl;
                cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
                cin.get();
            }
        }
        else
        {
            indexPertanyaanSaatIni++;
        }
    }

    cout << " Kuis selesai! Tekan Enter untuk melanjutkan..." << endl;
    cin.get();

    kuisSelesai(score, folderPath, selectedFileName);
}

void buatKuisBaru()
{
    system(CLEAR_SCREEN);
    banner("Buat Kuis Baru", "Kelompok 3", 10);
    // masukan nama file kemudian save di dalam folder ./questions
    // string namaFile;
    string judulKuis;
    string namaKategori;
    // string deskripsiKuis;

    cout << " Nama folder & file tidak boleh sama dengan file yang sudah ada" << endl;
    cout << " Jika nama file sudah ada, maka file akan ditimpa" << endl;
    cout << " Nama folder & file tidak boleh mengandung spasi" << endl;
    cout << " [X] Kembali" << endl;
    tampilan_pembatas(tampilan.jumlahPembatas);

    do
    {
        cout << " Masukkan nama kategori: ";
        getline(cin, namaKategori);
        // cin.ignore();

        if (namaKategori == "x" || namaKategori == "X")
        {
            mainMenuGuru();
            return; // Menghentikan eksekusi fungsi setelah kembali ke mainMenuGuru()
        }
    } while (namaKategori.empty()); // Menggunakan false sebagai kondisi perulangan agar perulangan hanya dilakukan sekali

    // cek apakah nama kategori sudah ada, jika sudah ada maka lanjutkan dengan nama kategori yang sama, jika tidak maka buat folder baru
    string kategoriPath = "./questions/" + namaKategori;
    if (fs::exists(kategoriPath))
    {
        // Folder already exists, do nothing
        namaKategori = namaKategori;
    }
    else
    {
        // Folder does not exist, create a new one
        fs::create_directory(kategoriPath);
    }
    // buat folder baru dengan namaKategori
    // string command = "mkdir ./questions/" + namaKategori; // linux
    // system(command.c_str());

    cout << " Nama kategori: " << namaKategori << endl;
    tampilan_pembatas(tampilan.jumlahPembatas);

    do
    {
        cout << " Masukkan judul kuis: ";
        getline(cin, judulKuis);
    } while (judulKuis.empty()); // Validasi judul kuis tidak boleh kosong

    // Membuat namaFile dengan mengganti spasi atau karakter yang tidak sesuai dengan penamaan file dengan "-"
    // for (char c : judulKuis)
    // {
    //     if (isalnum(c))
    //     {
    //         namaFile += c;
    //     }
    //     else if (isspace(c))
    //     {
    //         namaFile += "-";
    //     }
    // }
    // transform(namaFile.begin(), namaFile.end(), namaFile.begin(), ::tolower); // Mengubah semua huruf menjadi lowercase

    // cout << " Masukkan deskripsi: ";
    // getline(cin, deskripsiKuis);

    // // Jika deskripsi kosong, ganti dengan "-"
    // if (deskripsiKuis.empty())
    // {
    //     deskripsiKuis = "-";
    // }

    // while (namaFile.empty())
    // {
    //     cout << " Nama file tidak boleh kosong!" << endl;
    //     cout << " Masukkan nama file: ";
    //     cin >> namaFile;
    //     cin.ignore();
    // }

    cout << " Nama file: " << judulKuis << endl;

    // MENAMBAHKAN GRAPH KUIS
    graph.addQuiz(judulKuis, namaKategori);
    graph.saveQuizData();

    // buat file baru
    ofstream file("./questions/" + namaKategori + "/" + judulKuis);
    // masukan pertanyaan

    string pertanyaan;
    char jawaban;
    string pilihanJawaban;
    char pilihan;

    do
    {
        system(CLEAR_SCREEN);
        banner(judulKuis, "Kuis Baru", 10);

        cout << " Masukkan pertanyaan: ";
        getline(cin, pertanyaan);

        while (pertanyaan.empty())
        {
            cout << " Pertanyaan tidak boleh kosong!" << endl;
            cout << " Masukkan pertanyaan: ";
            getline(cin, pertanyaan);
        }

        // buat huruf pertama menjadi huruf besar
        pertanyaan[0] = toupper(pertanyaan[0]);
        file << pertanyaan << endl;
        // masukan pilihan
        for (int i = 0; i < 4; i++)
        {
            cout << " Masukkan pilihan " << (char)('A' + i) << ": ";
            getline(cin, pilihanJawaban);
            // jawaban tidak boleh kosong
            while (pilihanJawaban.empty())
            {
                cout << " Pilihan Jawaban tidak boleh kosong!" << endl;
                cout << " Masukkan pilihan " << (char)('A' + i) << ": ";
                getline(cin, pilihanJawaban);
            }
            // buat huruf pertama menjadi huruf besar
            pilihanJawaban[0] = toupper(pilihanJawaban[0]);
            file << (char)('A' + i) << ". " << pilihanJawaban << endl;
        }
        // masukan jawaban
        cout << " Masukkan jawaban yang benar (A, B, C, atau D): ";
        cin >> jawaban;
        cin.ignore();
        jawaban = toupper(jawaban);
        while (jawaban != 'A' && jawaban != 'B' && jawaban != 'C' && jawaban != 'D')
        {
            cout << " Jawaban tidak valid! Silahkan masukkan jawaban A, B, C, atau D" << endl;
            cout << " Masukkan jawaban yang benar: ";
            cin >> jawaban;
            cin.ignore();
            jawaban = toupper(jawaban);
        }
        file << "Jawaban: " << jawaban;
        // tanya apakah ingin menambah pertanyaan lagi?
        cout << " Apakah ingin menambah pertanyaan? (Y/N): ";
        cin >> pilihan;
        cin.ignore();
        pilihan = toupper(pilihan);
        while (pilihan != 'Y' && pilihan != 'N')
        {
            cout << " Pilihan tidak valid! Silahkan masukkan Y atau N" << endl;
            cout << " Apakah ingin menambah pertanyaan? (Y/N): ";
            cin >> pilihan;
            cin.ignore();
            pilihan = toupper(pilihan);
        }
        if (pilihan == 'Y')
        {
            file << endl;
        }
    } while (pilihan == 'Y');

    file.close();
}

void mainMenuGuru()
{
    string line;
    string namapengguna;

    ifstream file("./users/" + username + ".txt");

    int baris_saat_ini = 1;
    while (getline(file, line))
    {
        if (baris_saat_ini == barisDataUser.barisnama)
        {
            namapengguna = line;
            break;
        }
        baris_saat_ini++;
    }
    file.close();

    bool validInput = false;
    int pilihan;
    do
    {
        system(CLEAR_SCREEN);
        banner("SELAMAT DATANG DI", "ZONA KUIS - Kelompok 3", 10);

        cout << " Halo " << namapengguna << " mau apa hari ini?" << endl;
        cout << " Silahkan pilih menu:" << endl;
        cout << " [1] Buat Kuis Baru" << endl;
        cout << " [2] Export Soal ke CSV" << endl;
        cout << " [3] Riwayat Kuis" << endl;
        cout << " [4] Keluar Akun" << endl;
        cout << " [5] Keluar Aplikasi" << endl
             << endl;

        tampilan_pembatas(tampilan.jumlahPembatas);

        // validasi input
        // while (!validInput)
        // {
        cout << " Pilihan: ";
        string input;
        getline(cin, input);

        if (input.empty())
        {
            cout << " Pilihan tidak boleh kosong!" << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
            continue;
        }

        bool isNumeric = true;
        for (char c : input)
        {
            if (!isdigit(c))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric)
        {
            pilihan = stoi(input);
            if (pilihan >= 1 && pilihan <= 5)
            {
                validInput = true;
            }
            else
            {
                cout << " Pilihan tidak valid, silahkan pilih 1,2,3,4 atau 5!" << endl;
                cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
                cin.get();
            }
        }
        else
        {
            cout << " Pilihan tidak valid, silahkan pilih 1,2,3,4 atau 5!" << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
        }
        // }
    } while (!validInput);

    if (pilihan == 1)
    {
        buatKuisBaru();
    }
    else if (pilihan == 2)
    {
        exportSoalKeCSV();
    }
    else if (pilihan == 3)
    {
        riwayatPengerjaan();
    }
    else if (pilihan == 4)
    {
        cout << " Anda telah keluar dari akun guru..." << endl;
        cout << " Tekan tombol apapun untuk lanjut..." << endl;
        restartapp = 1;
        loggedRole = 0;
        cin.get();
        praMainMenu();
        return;
    }
    else if (pilihan == 5)
    {
        system(CLEAR_SCREEN);
        banner("Terima Kasih Sudah Menggunakan Aplikasi Kami!", "Kelompok 3 - PAMIT", 0);
        cout << " App Developed By: " << endl;
        cout << " KRISNA SANTOSA           - 2209092" << endl;
        cout << " RAFLY IVAN KHALFANI      - 2202187" << endl;
        cout << " RAMANDHA PUTRA SURYAHADI - 2200125" << endl;
        cout << " ";
        tampilan_pembatas(tampilan.jumlahPembatas);
        cout << " Tekan tombol apapun untuk keluar..." << endl;
        cin.get();
        exit(0);
    }
    else
    {
        cout << " Pilihan tidak valid!" << endl;
        cout << " Tekan tombol apapun untuk kembali ke menu utama..." << endl;
        cin.get();
        cin.get();
        // clear the console
        system(CLEAR_SCREEN);
    }

    mainMenuGuru();
}

void mainMenuSiswa()
{
    string line;
    string namapengguna;

    ifstream file("./users/" + username + ".txt");
    int baris_saat_ini = 1;
    while (getline(file, line))
    {
        if (baris_saat_ini == barisDataUser.barisnama)
        {
            namapengguna = line;
            break;
        }
        baris_saat_ini++;
    }
    file.close();

    int pilihan;
    // validasi input
    bool validInput = false;
    do
    {
        system(CLEAR_SCREEN);
        banner("SELAMAT DATANG DI", "ZONA KUIS - Kelompok 3", 10);

        cout << " Halo " << namapengguna << " mau apa hari ini?" << endl;
        cout << " Silahkan pilih menu:" << endl;
        cout << " [1] Mulai Kuis" << endl;
        cout << " [2] Riwayat Kuis" << endl;
        cout << " [3] Cara Mengerjakan Kuis" << endl;
        cout << " [4] Keluar Akun" << endl;
        cout << " [5] Keluar Aplikasi" << endl
             << endl;
        tampilan_pembatas(tampilan.jumlahPembatas);

        // while (!validInput)
        // {
        cout << " Pilihan: ";
        string input;
        getline(cin, input);

        if (input.empty())
        {
            cout << " Pilihan tidak boleh kosong!" << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
            continue;
        }

        bool isNumeric = true;
        for (char c : input)
        {
            if (!isdigit(c))
            {
                isNumeric = false;
                break;
            }
        }

        if (isNumeric)
        {
            pilihan = stoi(input);
            if (pilihan >= 1 && pilihan <= 5)
            {
                validInput = true;
            }
            else
            {
                cout << " Pilihan tidak valid, silahkan pilih 1,2,3 atau 4!" << endl;
                cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
                cin.get();
            }
        }
        else
        {
            cout << " Pilihan tidak valid, silahkan pilih 1,2,3 atau 4!" << endl;
            cout << " Tekan tombol apapun untuk melanjutkan..." << endl;
            cin.get();
        }
        // }
    } while (!validInput);

    if (pilihan == 1)
    {
        mainkanKuis();
    }
    else if (pilihan == 2)
    {
        riwayatPengerjaan();
    }
    else if (pilihan == 3)
    {
        // system(CLEAR_SCREEN);
        caraBermain();
        cout << endl;
        cout << " Tekan tombol apapun untuk kembali ke menu utama..." << endl;
        cin.get();

        mainMenuSiswa();
    }
    else if (pilihan == 4)
    {
        cout << " Anda telah keluar dari akun siswa..." << endl;
        cout << " Tekan tombol apapun untuk lanjut..." << endl;
        loggedRole = 0;
        restartapp = 1;
        cin.get();
        praMainMenu();
        return;
    }
    else if (pilihan == 5)
    {
        system(CLEAR_SCREEN);
        banner("Terima Kasih Sudah Menggunakan Aplikasi Kami!", "Kelompok 3 - PAMIT", 0);
        cout << " App Developed By: " << endl;
        cout << " KRISNA SANTOSA           - 2209092" << endl;
        cout << " RAFLY IVAN KHALFANI      - 2202187" << endl;
        cout << " RAMANDHA PUTRA SURYAHADI - 2200125" << endl;
        cout << " ";
        tampilan_pembatas(tampilan.jumlahPembatas);
        cout << " Tekan tombol apapun untuk keluar..." << endl;
        cin.get();
        exit(0);
    }
    else
    {
        cout << " Pilihan tidak valid!" << endl;
        cout << " Tekan tombol apapun untuk kembali ke menu utama..." << endl;
        cin.get();
        cin.get();
        // clear the console
        system(CLEAR_SCREEN);
    }
    mainMenuSiswa();
}

int main()
{
    bool done = false;
    if (!done)
    {
        graph.addPersonFromFile();
        graph.addQuizFromFile();
        graph.addQuizHistoryFromFile();

        done = true;
    }

    do
    {
        while (restartapp == 0 || restartapp == 1)
        {
            system(CLEAR_SCREEN);
            restartapp = 2;
            praMainMenu();
        }
        system(CLEAR_SCREEN);

        praMainMenu();
    } while (restartapp == 1);

    return 0;
}