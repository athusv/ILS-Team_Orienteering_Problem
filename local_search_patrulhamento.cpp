#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <queue>
#include <random>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <cassert>
#include <map>
#include <iterator>
#include "Instance.h"
#include "Caminho.h"
#include "Sol.h"
#include "Utils.h"
#include "Perturbacao.h"
#include "Busca_local.h"

#define NDEBUG

using namespace std;

template<typename T>
priority_queue<T> join_priority_queues(priority_queue<T> q1, priority_queue<T> q2) {
    priority_queue<T> result; // Nova lista de prioridade

    // Popule a nova lista com os elementos de q1
    while (!q1.empty()) {
        result.push(q1.top());
        q1.pop();
    }

    // Popule a nova lista com os elementos de q2
    while (!q2.empty()) {
        result.push(q2.top());
        q2.pop();
    }

    return result;
}

void Guloso(Instance &grafo, Sol &s0) {
    // Variavel de parada do while, todas as rotas estao prontas
    priority_queue<Caminho> rotas_prontas;
    while (!s0.rotas.empty()) {
        Caminho rota = s0.rotas.top();
        s0.rotas.pop();
        vector<int> lista_candidatos = Utils::make_lista(grafo, rota, s0.visited_vertices);
        if (lista_candidatos.size() == 0) {
            rota.custo += grafo.distancia_matriz[rota.route.back()][0];
            rota.route.push_back(0);
            rota.paradas.push_back(0);
            rota.visita_custo.push_back(rota.custo);
            rota.push_hotspots.push_back({999, 999});

            rota.atualizar_push_hotspots(s0.visited_vertices);
            rotas_prontas.push(rota);
            s0.custo += rota.custo;
            s0.score += rota.score;
            // rota.print_push();
            continue;
        }

        int index_max = Utils::max_score(lista_candidatos, grafo.score_vertices);

        rota.custo += grafo.distancia_matriz[rota.route.back()][index_max] + rota.plus_parada;
        double s = grafo.score_vertices[index_max];

        if (rota.plus_parada == grafo.t_parada)
        {
            s = grafo.score_vertices[index_max];
            rota.score += s;
            rota.paradas.push_back(1);
        }
        else
        {
            s = grafo.score_vertices[index_max]/3;
            rota.score += s;
            rota.paradas.push_back(0);
        }
        rota.visita_custo.push_back(rota.custo);
        rota.route.push_back(index_max);
        rota.push_hotspots.push_back({999, 999});

        s0.visited_vertices[index_max][rota.custo + grafo.t_prot] = rota.id;

        if (rota.z == 0) {
            rota.plus_parada = 0;
            rota.z += 1;
        } else if (rota.z == 1) {
            rota.z += 1;
        } else if (rota.z == 2) {
            rota.plus_parada = grafo.t_parada;
            rota.z = 0;
        }

        // cout << rota << endl;

        s0.rotas.push(rota);
    }
    s0.rotas = rotas_prontas;
}

// vector<double> passa(const Instance &grafo, vector<map<double, int> > &visited_vertices, const Caminho &rota) {
//     // se tiver uma pontuaçao pelo menos 10% abaixo da media, para, se nao, passa
//     // int media = rota.score / (rota.route.size() - 2);
//     // media = media - media * 0.1;
//     // colocar para passar o menor vertice
//     vector<double> vertice_passa = {-1, -1, -1};

//     for (int i = 1; i < rota.route.size() - 1; i++) {
//         if (rota.paradas[i] == 0)
//             continue;

//         if (grafo.score_vertices[rota.route[i]] >= vertice_passa[2])
//             continue;

//         bool possibilidade_passada = true;

//         for (int a = i; a < rota.route.size() - 1; a++) {
//             auto it = visited_vertices[rota.route[a]].find(rota.visita_custo[a] + grafo.t_prot);
//             auto it_prox = next(it);
//             auto it_ant = prev(it);
//             if (it_prox != visited_vertices[rota.route[a]].end() && it->first - grafo.t_parada > it_prox->first - grafo.t_prot)
//             {
//                 possibilidade_passada = false;
//                 break;
//             }
//             if (it_ant != visited_vertices[rota.route[a]].end() && it->first - grafo.t_parada - grafo.t_prot < it_ant->first)
//             {
//                 possibilidade_passada = false;
//                 break;
//             }
//         }

//         if (possibilidade_passada) {
//             //                   pontuaçao_parcial                             pontuaçao completa

//             vertice_passa = {i, grafo.score_vertices[rota.route[i]]/3, grafo.score_vertices[rota.route[i]]};
//         }
//     }

//     return vertice_passa;
// }

Sol ILS(Sol &s0, Instance &grafo, mt19937 gen, double tempo_maximo)
{
    // cout << "ILS" << endl;
    Sol s = s0; Sol s1 = s0;
    Sol best_s = s0;
    string chamou;

    auto inicio = std::chrono::high_resolution_clock::now();
    int i = 1;
    while(true) {
        s1 = s;
        s1 = Perturbacao::perturbacao(grafo, s1, gen);
        s1.atualiza_push(grafo);
        chamou = "Pertubação";
        s1.checa_solucao(grafo, chamou);
        // cout << "|||||||||||Pertubação " << i << endl;
        // cout << "Score: " << s1.score << ", Custo: " << s1.custo << endl;

        s1 = Busca_local::busca_local(grafo, s1, gen);
        s1.atualiza_push(grafo);

        chamou = "Busca Local";
        s1.checa_solucao(grafo, chamou);
        // cout << "||||||||||||Busca Local " << i << endl;
        // cout << "Score: " << s1.score << ", Custo: " << s1.custo << endl;
        if (!Utils::doubleGreaterOrEqual(best_s.score, s1.score))
        {

            // cout << "Melhor solucão encontrada ***** " << i << endl;
            // cout << "Score: " << s1.score << ", Custo: " << s1.custo << endl;
            best_s = s1;
            s = best_s;
        }
        // Verifica se o tempo máximo foi atingido
        auto agora = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duracao = agora - inicio;

        if (duracao.count() >= tempo_maximo)
        {
            // std::cout << "Tempo máximo de execução atingido: " << duracao.count() << " segundos.\n";
            std::cout << "Iteração: "<<i<<"\n";
            break;
        }

        i++;
    }

    return best_s;
}

int main(int argc, char *argv[])
{
    if(argc < 4){
        cout << "coloque todas as informaçoes necessarias!" <<endl;
        return 1;
    }
    int t_prot = stoi(argv[2]);
    int t_parada = stoi(argv[3]);
    double velocidade = stod(argv[4]);
    // Lendo grafo
    string instancia = string(argv[1]);

    // Instance grafo("C:/Users/athus/Faculdade/6 periodo/PIBIT/solucao/pibit-rotas-pm/" + instancia);
    Instance grafo(instancia, t_prot, t_parada, velocidade); //nome da instancia, t_prot, t_parada

    // Solução inicial
    Sol s0(grafo);
    // cout<<"Guloso"<<endl;
    Guloso(grafo, s0);
    s0.atualiza_push(grafo);
    std::string chamou = "Guloso";
    s0.checa_solucao(grafo, chamou);

    unsigned int seed_value;
    std::random_device rd;

    if (argc > 5){
        seed_value = stoi(argv[5]);
        cout << "Seed fornecida: " << seed_value << std::endl;
    }else{
        random_device rd;
        seed_value = rd();
        cout << "Seed aleatória gerada: " << seed_value << std::endl;
    }
    mt19937 gen(seed_value);

    double tempo_maximo = 5.0; //Em segundos
    Sol s1 = ILS(s0, grafo, gen, tempo_maximo);

    double percentual_melhora = (static_cast<double>(s1.score - s0.score) / s0.score) * 100;
    // std::cout << "Tempo Máximo: " << grafo.tmax * grafo.veiculos << std::endl;
    cout << "Seed: "<<seed_value<< " | T_prot: "<< t_prot << "min | T_parada: " <<t_parada<< "min | Velocidade: "<<velocidade<<"Km/h"<< endl;
    cout << "Instância: " << instancia << " | Vértices: " <<grafo.qt_vertices << " | Veículos: " <<grafo.veiculos<<endl;
    cout << "Solução Gulosa - Score: "<<s0.score<<" | Custo: " << s0.custo<<endl;
    cout << "Solução ILS - Score: " << s1.score << " | Custo: "<<s1.custo<<endl;
    cout << "Melhoria de " << percentual_melhora << "%"<<endl<<endl;
    // s0.print_solucao(grafo);
}