#include <sys/stat.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;


int ordemGlobal = 3;     

int nodeCount = 0;

struct Tupla {
  int id;
  string rotulo;
  int ano;
  string tipo;
};

struct Node {
  int id;
  bool isLeaf;
  vector<int> keys;
  vector<Node*> children;
  vector<vector<int>> refs;  
  Node* parent;
  Node* next;  
  Node* prev;  

  Node(bool leaf = false)
      : id(++nodeCount), isLeaf(leaf), parent(nullptr), next(nullptr), prev(nullptr) {}

 
  string serialize() {
    ostringstream ss;
    ss << id << "," << (isLeaf ? "'folha'" : "'interno'") << ",[";
    for (size_t i = 0; i < keys.size(); ++i) {
      ss << keys[i];
      if (i + 1 < keys.size()) ss << ",";
    }
    ss << "]," << (parent ? to_string(parent->id) : "null") << ",";

    if (isLeaf) {
      ss << (prev ? to_string(prev->id) : "null") << "," << (next ? to_string(next->id) : "null") << ",";
      for (size_t i = 0; i < refs.size(); ++i) {
        ss << "[";
        for (size_t j = 0; j < refs[i].size(); ++j) {
          ss << refs[i][j];
          if (j + 1 < refs[i].size()) ss << ",";
        }
        ss << "]";
        if (i + 1 < refs.size()) ss << ",";
      }
    } else {
      ss << "[";
      for (size_t i = 0; i < children.size(); ++i) {
        ss << children[i]->id;
        if (i + 1 < children.size()) ss << ",";
      }
      ss << "]";
    }
    return ss.str();
  }
};

bool arquivoExiste(const string& nome) {
  struct stat buffer;
  return (stat(nome.c_str(), &buffer) == 0);
}

void escreverNoIndice(Node* node, const string& pathIndice) {
  ifstream original;
  if (arquivoExiste(pathIndice)) {
    original.open(pathIndice);
  }
  ofstream temp("temp.txt");
  string linha;
  bool escrito = false;

  if (original.is_open()) {
    while (getline(original, linha)) {
      stringstream ss(linha);
      string idStr;
      getline(ss, idStr, ',');
      int linhaID = stoi(idStr);
      if (linhaID == node->id) {
        temp << node->serialize() << "\n";
        escrito = true;
      } else {
        temp << linha << "\n";
      }
    }
    original.close();
  }

  if (!escrito) {
    temp << node->serialize() << "\n";
  }

  temp.close();
  remove(pathIndice.c_str());
  rename("temp.txt", pathIndice.c_str());
}
class BPlusTree {
 public:
  BPlusTree(const string& path, int ordem) : pathIndice(path), ordem(ordem) {
    root = new Node(true);
    height = 1;
    escreverNoIndice(root, pathIndice);
  }


  Node* getRoot() const { return root; }


  Node* findLeaf(Node* node, int key) {
    while (!node->isLeaf) {
      auto it = upper_bound(node->keys.begin(), node->keys.end(), key);
      int idx = distance(node->keys.begin(), it);
      node = node->children[idx];
    }
    return node;
  }

  void insert(int key, const vector<int>& ids) {
    Node* leaf = findLeaf(root, key);
    insertIntoLeaf(leaf, key, ids);
  }

  int getHeight() const { return height; }
  vector<int> search(int key) {
    Node* leaf = findLeaf(root, key);
    auto it = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int idx = distance(leaf->keys.begin(), it);
    if (it != leaf->keys.end() && *it == key) {
      return leaf->refs[idx];  
    }
    return {};  
  }

 private:
   int ordem;

  Node* root;
  int height;
  string pathIndice;

  void insertIntoLeaf(Node* leaf, int key, const vector<int>& ids) {
    auto it = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int idx = distance(leaf->keys.begin(), it);


    if (it != leaf->keys.end() && *it == key) {
      return;
    }

    leaf->keys.insert(it, key);
    leaf->refs.insert(leaf->refs.begin() + idx, ids);

    if ((int)leaf->keys.size() > ordem - 1) {
      splitLeaf(leaf);
    } else {
      escreverNoIndice(leaf, pathIndice);
    }
  }

  void splitLeaf(Node* leaf) {
    int mid = ordem / 2;

    Node* newLeaf = new Node(true);
    newLeaf->parent = leaf->parent;

    newLeaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    newLeaf->refs.assign(leaf->refs.begin() + mid, leaf->refs.end());

    leaf->keys.resize(mid);
    leaf->refs.resize(mid);

    newLeaf->next = leaf->next;
    if (leaf->next) leaf->next->prev = newLeaf;
    leaf->next = newLeaf;
    newLeaf->prev = leaf;

    escreverNoIndice(leaf, pathIndice);
    escreverNoIndice(newLeaf, pathIndice);

    int promotedKey = newLeaf->keys.front();

    insertIntoParent(leaf, promotedKey, newLeaf);
  }

void splitInternal(Node* node) {
    int mid = (ordem - 1) / 2; 
    int promoteKey = node->keys[mid];

    Node* newNode = new Node(false);
    newNode->parent = node->parent;


    newNode->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    newNode->children.assign(node->children.begin() + mid + 1, node->children.end());

    for (Node* child : newNode->children) {
        child->parent = newNode;
    }

    node->keys.resize(mid);
    node->children.resize(mid + 1);

    escreverNoIndice(node, pathIndice);
    escreverNoIndice(newNode, pathIndice);

  
    if (node->parent == nullptr) {
        Node* newRoot = new Node(false);
        newRoot->keys = {promoteKey};
        newRoot->children = {node, newNode};
        node->parent = newRoot;
        newNode->parent = newRoot;
        root = newRoot;
        height++;
        escreverNoIndice(newRoot, pathIndice);
    } else {
        insertIntoParent(node, promoteKey, newNode);
    }
}




void insertIntoParent(Node* left, int key, Node* right) {
    Node* parent = left->parent;


    if (parent == nullptr) {
        Node* newRoot = new Node(false);
        newRoot->keys = {key};
        newRoot->children = {left, right};
        left->parent = newRoot;
        right->parent = newRoot;
        root = newRoot;
        height++;
        escreverNoIndice(newRoot, pathIndice);
        return;
    }


    auto it = std::find(parent->children.begin(), parent->children.end(), left);
    if (it == parent->children.end()) {
        std::cerr << "Erro: nó filho não encontrado no pai durante insertIntoParent.\n";
        return;
    }

    int idx = std::distance(parent->children.begin(), it);


    parent->keys.insert(parent->keys.begin() + idx, key);
    parent->children.insert(parent->children.begin() + idx + 1, right);
    right->parent = parent;

    escreverNoIndice(parent, pathIndice);

    if ((int)parent->keys.size() > ordem - 1) {
        splitInternal(parent);
    }
}

};

Tupla parseLinhaCSV(const string& linha) {
  stringstream ss(linha);
  string item;
  Tupla t;
  getline(ss, item, ',');
  t.id = stoi(item);
  getline(ss, t.rotulo, ',');
  getline(ss, item, ',');
  t.ano = stoi(item);
  getline(ss, t.tipo, ',');
  return t;
}


vector<int> buscarIDsPorAno(const string& arquivoCSV, int ano) {
  ifstream f(arquivoCSV);
  string linha;
  getline(f, linha);  
  vector<int> resultado;
  while (getline(f, linha)) {
    Tupla t = parseLinhaCSV(linha);
    if (t.ano == ano) resultado.push_back(t.id);
  }
  return resultado;
}

int main() {
  string pathIn = "in.txt";
  string pathOut = "out.txt";
  string pathCSV = "vinhos.csv";
  string pathIndice = "indice.txt";

  ifstream in(pathIn);
  ofstream out(pathOut);
  if (!in.is_open() || !out.is_open()) {
    cerr << "Erro ao abrir arquivos in.txt ou out.txt\n";
    return 1;
  }


  string linhaInicial;
  getline(in, linhaInicial);
  out << linhaInicial << "\n";

  int ordem = 3; 
  if (linhaInicial.rfind("FLH/", 0) == 0) {
    ordem = stoi(linhaInicial.substr(4));
  } else {
    cerr << "Linha inicial inválida. Esperado: FLH/<ordem>\n";
    return 1;
  }


  BPlusTree tree(pathIndice, ordem);

 
  string comando;
  while (getline(in, comando)) {
    if (comando.rfind("INC:", 0) == 0) {
      size_t pos = comando.find('/');
      int val = stoi(comando.substr(4, pos - 4));

      auto ids = buscarIDsPorAno(pathCSV, val);

      Node* leaf = tree.findLeaf(tree.getRoot(), val);
      bool exists = std::find(leaf->keys.begin(), leaf->keys.end(), val) != leaf->keys.end();

      if (!exists) {
        tree.insert(val, ids);
        out << "INC:" << val << "/" << ids.size() << "\n";
      } else {
        out << "INC:" << val << "/0\n";
      }

    } else if (comando.rfind("BUS=:", 0) == 0) {
      size_t pos = comando.find('/');
      int val = stoi(comando.substr(5, pos - 5));

      auto ids = tree.search(val);
      out << "BUS=:" << val << "/" << ids.size() << "\n";
    }
  }

  in.close();
  out.close();
  return 0;
}

