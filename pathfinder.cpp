#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <stack>
#include <random>

using namespace std;

class Celda {
public:
    int x, y;
    bool visitado;
    bool N, S, E, O;

    Celda(int x, int y) : x(x), y(y), visitado(false), N(true), S(true), E(true), O(true) {}
};

class Laberinto {
private:
    int columnas, filas;
    vector<vector<Celda>> grilla;

public:
    Laberinto(int columnas, int filas) : columnas(columnas), filas(filas) {
        for (int x = 0; x < filas; ++x) {
            vector<Celda> fila;
            for (int y = 0; y < columnas; ++y) {
                fila.emplace_back(x, y);
            }
            grilla.push_back(fila);
        }
    }

    void mostrar() const {
        for (int x = 0; x < filas; ++x) {
            for (int y = 0; y < columnas; ++y) {
                const Celda& celda = grilla[x][y];
                cout << (celda.N ? "+---" : "+   ");
            }
            cout << "+" << endl;
            for (int y = 0; y < columnas; ++y) {
                const Celda& celda = grilla[x][y];
                cout << (celda.O ? "|   " : "    ");
            }
            cout << "|" << endl;
        }
        for (int y = 0; y < columnas; ++y) {
            cout << "+---";
        }
        cout << "+" << endl;
    }

    void mostrarConSolucion(const vector<pair<int, int>>& solucion) const {
        for (int x = 0; x < filas; ++x) {
            for (int y = 0; y < columnas; ++y) {
                const Celda& celda = grilla[x][y];
                //Si existe muro al N, imprimir esquina y muro, caso contrario, solo la esquina
                cout << (celda.N ? "+---" : "+   ");
            }
            cout << "+" << endl;
            for (int y = 0; y < columnas; ++y) {
                const Celda& celda = grilla[x][y];
                if (find(solucion.begin(), solucion.end(), make_pair(x, y)) != solucion.end()) {
                    cout << (celda.O ? "| * " : "  * ");
                } else {
                    cout << (celda.O ? "|   " : "    ");
                }
            }
            cout << "|" << endl;
        }
        for (int y = 0; y < columnas; ++y) {
            cout << "+---";
        }
        cout << "+" << endl;
    }
    // Permitir a estas clases acceder a los miembros privados
    friend class Generador;
    friend class Resolucion;
};

class Generador {
private:
    Laberinto& laberinto;
    random_device rd;
    mt19937 gen;
    uniform_int_distribution<> dis;

    //Devolver celdas adyacentes no visitadas
    vector<pair<int, int>> obtenerVecinos(Celda& celda) {
        vector<pair<int, int>> vecinos;
        int x = celda.x;
        int y = celda.y;

        if (x > 0 && !laberinto.grilla[x - 1][y].visitado)
            vecinos.emplace_back(x - 1, y);
        if (x < laberinto.filas - 1 && !laberinto.grilla[x + 1][y].visitado)
            vecinos.emplace_back(x + 1, y);
        if (y > 0 && !laberinto.grilla[x][y - 1].visitado)
            vecinos.emplace_back(x, y - 1);
        if (y < laberinto.columnas - 1 && !laberinto.grilla[x][y + 1].visitado)
            vecinos.emplace_back(x, y + 1);

        //Mezclar las referencias en el array
        shuffle(vecinos.begin(), vecinos.end(), gen);
        return vecinos;
    }

    void removerPared(Celda& actual, Celda& vecino) {
        if (actual.x == vecino.x) {
            if (actual.y < vecino.y) {
                actual.E = false;
                vecino.O = false;
            } else {
                actual.O = false;
                vecino.E = false;
            }
        } else {
            if (actual.x < vecino.x) {
                actual.S = false;
                vecino.N = false;
            } else {
                actual.N = false;
                vecino.S = false;
            }
        }
    }

public:
    Generador(Laberinto& laberinto) : laberinto(laberinto), gen(rd()), dis(0, 1) {}

    void generar() {
        //Stack de celdas
        stack<Celda*> stack;
        Celda& celdaInicial = laberinto.grilla[0][0];
        celdaInicial.visitado = true;
        stack.push(&celdaInicial);

        while (!stack.empty()) {
            //Mientras que el stack se no se encuentre vacio, agarrar el primer puntero
            Celda* celdaActual = stack.top();
            //Obtener las celdas adyacentes
            vector<pair<int, int>> vecinos = obtenerVecinos(*celdaActual);

            //Si existen celdas adyacentes no visitadas
            if (!vecinos.empty()) {
                //Los punteros del array estan no estan en orden por lo que se respeta aleatoriedad
                auto [x, y] = vecinos[0];
                //Asignar el valor del puntero a un objeto
                Celda& celdaVecina = laberinto.grilla[x][y];
                //Remover pared entre la celda actual y la celda contigua
                removerPared(*celdaActual, celdaVecina);
                //Marcar celda actuaal como visitada y agregarla al stack
                celdaVecina.visitado = true;
                stack.push(&celdaVecina);
            } else {
                //Backtrack en caso de que todas las celdas adyacentes esten visitadas
                stack.pop();
            }
        }

        //Restablecer para la resolucion
        for (auto& fila : laberinto.grilla) {
            for (auto& celda : fila) {
                celda.visitado = false;
            }
        }
    }
};

class Resolucion {
private:
    Laberinto& laberinto;
    vector<pair<int, int>> solucion;

    //Backtracking recursivo
    bool backtrack(Celda& celda, const pair<int, int>& objetivo) {
        if (celda.x == objetivo.first && celda.y == objetivo.second) {
            solucion.emplace_back(celda.x, celda.y);
            return true;
        }

        celda.visitado = true;
        vector<Celda*> vecinos;

        if (!celda.N && !laberinto.grilla[celda.x - 1][celda.y].visitado)
            vecinos.push_back(&laberinto.grilla[celda.x - 1][celda.y]);
        if (!celda.S && !laberinto.grilla[celda.x + 1][celda.y].visitado)
            vecinos.push_back(&laberinto.grilla[celda.x + 1][celda.y]);
        if (!celda.O && !laberinto.grilla[celda.x][celda.y - 1].visitado)
            vecinos.push_back(&laberinto.grilla[celda.x][celda.y - 1]);
        if (!celda.E && !laberinto.grilla[celda.x][celda.y + 1].visitado)
            vecinos.push_back(&laberinto.grilla[celda.x][celda.y + 1]);

        for (auto vecino : vecinos) {
            if (backtrack(*vecino, objetivo)) {
                solucion.emplace_back(celda.x, celda.y);
                return true;
            }
        }

        return false;
    }

public:
    Resolucion(Laberinto& laberinto) : laberinto(laberinto) {}

    vector<pair<int, int>> resolver() {
        solucion.clear();
        Celda& celdaInicial = laberinto.grilla[0][0];
        pair<int, int> objetivo = {laberinto.filas - 1, laberinto.columnas - 1};

        if (backtrack(celdaInicial, objetivo)) {
            reverse(solucion.begin(), solucion.end());
        }

        return solucion;
    }
};

int main() {
    int columnas = 20;
    int filas = 20;

    Laberinto laberinto(columnas, filas);
    Generador generador(laberinto);
    Resolucion resolucion(laberinto);

    generador.generar();
    laberinto.mostrar();

    vector<pair<int, int>> solucion = resolucion.resolver();
    cout << "Solucion:" << endl;
    for (const auto& paso : solucion) {
        cout << "(" << paso.first << ", " << paso.second << ")" << endl;
    }

    laberinto.mostrarConSolucion(solucion);

    return 0;
}
