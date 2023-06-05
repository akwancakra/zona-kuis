#include <iostream>
#include <string>
#include <fstream>

using namespace std;

const int MAX_TITLE_LENGTH = 50;
const int MAX_CATEGORY_LENGTH = 50;
const int MAX_ADJACENCYLIST_LENGTH = 50;
const int MAX_HISTORYQUIZ_LENGTH = 50;
const int MAX_SCORE_LENGTH = 50;

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
        ofstream file("./questions/riwayat/data-user.csv");

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
        ofstream file("./questions/riwayat/data-kuis.csv");

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
        ofstream file("./questions/riwayat/riwayat.csv");

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
        ifstream file("./questions/riwayat/data-user.csv");

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
        ifstream file("./questions/riwayat/data-kuis.csv");

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
        ifstream file("./questions/riwayat/riwayat.csv");

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
            Quiz *quiz = new Quiz(quizTitle, categories[quizIndex], score);
            adjacencyList[personIndex]->addQuiz(quiz);
        }
    }

    void printGraph()
    {
        for (int i = 0; i < numVertices; i++)
        {
            Node *current = adjacencyList[i];
            if (current != nullptr)
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
        }
    }
};

int main()
{
    int numPeople = 50;

    Graph graph(numPeople);

    // graph.addPerson("Akwan Cakra");
    // graph.addPerson("John Doe");

    // graph.addQuiz("Probstat 2C", "Probabilitas");
    // graph.addQuiz("Matematika Diskrit 2C", "Matdis");

    // graph.addQuizHistory("John Doe", "Probstat 2C", "4/5");
    // graph.addQuizHistory("John Doe", "Matematika Diskrit 2C", "6/10");
    // graph.addQuizHistory("Akwan Cakra", "Probstat 2C", "5/5");

    graph.addPersonFromFile();
    graph.addQuizFromFile();
    graph.addQuizHistoryFromFile();

    // graph.addQuizHistory("Akwan Cakra", "Matematika Diskrit 2C", "7/7");

    graph.printGraph();

    graph.saveUserData();
    graph.saveQuizData();
    graph.saveQuizHistoryData();

    return 0;
}

// #include <iostream>
// #include <vector>

// using namespace std;

// class Node
// {
// public:
//     string name;
//     vector<pair<string, pair<string, string>>> history;

//     Node(const string &n)
//     {
//         name = n;
//     }
// };

// class Graph
// {
// private:
//     int numVertices;
//     vector<Node *> adjacencyList;

// public:
//     Graph(int vertices)
//     {
//         numVertices = vertices;
//         adjacencyList.resize(numVertices, nullptr);
//     }

//     void addPerson(int person, const string &name)
//     {
//         Node *node = new Node(name);
//         adjacencyList[person] = node;
//     }

//     void addQuizHistory(int person, const string &quiz, const string &category, const string &score)
//     {
//         if (adjacencyList[person] != nullptr)
//         {
//             adjacencyList[person]->history.push_back(make_pair(quiz, make_pair(category, score)));
//         }
//     }

//     void printGraph()
//     {
//         for (int i = 0; i < numVertices; i++)
//         {
//             Node *current = adjacencyList[i];
//             if (current != nullptr)
//             {
//                 cout << "----" << current->name << "----" << endl;
//                 for (const auto &quiz : current->history)
//                 {
//                     cout << "Kuis: " << quiz.first << endl;
//                     cout << "Kategori: " << quiz.second.first << endl;
//                     cout << "Skor: " << quiz.second.second << endl
//                          << endl;
//                 }
//             }
//         }
//     }
// };

// int main()
// {
//     int numPeople = 2;

//     Graph graph(numPeople);

//     // Menambahkan simpul orang
//     graph.addPerson(0, "Akwan Cakra");
//     graph.addPerson(1, "John Doe");

//     // Menambahkan hubungan simpul orang dengan simpul riwayat kuis
//     graph.addQuizHistory(0, "Matematika Diskrit 2C", "Matdis", "2/7");
//     graph.addQuizHistory(1, "Probstat 2C", "Probabilitas", "4/5");
//     graph.addQuizHistory(0, "Probstat 2C", "Probabilitas", "5/5");
//     graph.addQuizHistory(1, "Fisika Dasar 2C", "Fisika", "6/10");

//     // Menampilkan graf
//     graph.printGraph();

//     return 0;
// }

// #include <iostream>
// #include <vector>

// using namespace std;

// class Graph
// {
// private:
//     int numVertices;
//     vector<vector<int>> adjacencyList;

// public:
//     Graph(int vertices)
//     {
//         numVertices = vertices;
//         adjacencyList.resize(numVertices);
//     }

//     void addEdge(int src, int dest)
//     {
//         adjacencyList[src].push_back(dest);
//         adjacencyList[dest].push_back(src); // Tambahkan edge ke kedua verteks terhubung
//     }

//     void printGraph()
//     {
//         for (int i = 0; i < numVertices; i++)
//         {
//             cout << "Vertex " << i << ": ";
//             for (int dest : adjacencyList[i])
//             {
//                 cout << dest << " ";
//             }
//             cout << endl;
//         }
//     }
// };

// int main()
// {
//     int numVertices = 5;
//     Graph graph(numVertices);

//     // Menambahkan edge/relasi antar verteks
//     graph.addEdge(0, 1);
//     graph.addEdge(0, 4);
//     graph.addEdge(1, 2);
//     graph.addEdge(1, 3);
//     graph.addEdge(1, 4);
//     graph.addEdge(2, 3);
//     graph.addEdge(3, 4);

//     // Menampilkan graf
//     graph.printGraph();

//     return 0;
// }
