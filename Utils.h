#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <vector>
#include <map>
#include "Caminho.h"
#include "Instance.h"
#include "Sol.h"

class Utils {
public:
    static bool doubleEquals(double a, double b, double epsilon = 1e-9);
    static bool doubleGreaterOrEqual(double a, double b, double epsilon = 1e-9);
    static bool doubleLessOrEqual(double a, double b, double epsilon = 1e-9);
    static int max_score(const std::vector<int>& lista_candidatos, const std::vector<double>& score_vertices);
    static std::vector<int> make_lista(Instance& grafo, Caminho& rota, std::vector<std::map<double, int>>& visited_vertices);
    static std::vector<double> p_excluir(Instance& grafo, std::vector<std::map<double, int>>& visited_vertices, Caminho& rota, int i);
    static vector<vector<double>> swap_perturbacao(Instance &grafo, Sol &s0, Caminho &rota, int i);
    static std::vector<double> p_insert(Instance& grafo, std::vector<std::map<double, int>>& visited_vertices, const Caminho& rota, int i, int vertice_insert);
};

#endif // UTILS_H

