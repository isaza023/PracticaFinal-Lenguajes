/* =========================================================================
   PRACTICA 2  | Acerca de El metro integrado y itinerario
   Hecho por : Samuel Gómez y Samuel Isaza
   Medellin Metro Sistema en C++ - ST0244 EAFIT
   Compila con: g++ -std=c++17 -Wall -Wextra main.cpp
   Para C++ 17 y C++ 23
   ========================================================================= */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <map>
#include <functional>

/* -------------------------------------------------------------------------
   Esta parte es para el formato en COP x ej (3820 -> $3820)
   ------------------------------------------------------------------------- */
std::string formatCOP(long long value) { /* 3820 entra en long long y sale string */
    std::string digits = std::to_string(value); /* Para ponerle . hay que convertir a texto */
    std::string out; 
    int count = 0; /* El contador empieza en cero */
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) { // De der. a izq se lee porque se trata de plata
        out.push_back(*it); // Agarra el numero actual y lo pega en out
        if (++count % 3 == 0 && it + 1 != digits.rend()) { //Quiere decir cada 3 digitos el %3 == 0
            out.push_back('.'); //Pone el punto si quedan más numeros por leer
        } // Hasta aqui esta como 0.28.3
    }
    std::reverse(out.begin(), out.end()); //aqui voltea para que quede bien
    return "$ " + out;  //En $3.820
} //FormatCOP es una funcion auxiliar

   //La clase que sostiene todo, la base TransportMode (Bloque 1)
   
class TransportMode {
public:
    TransportMode(std::string name, int stations) //Es el constructor que recibe el nombre, # de estaciones y las guarda
        : name_(std::move(name)), stations_(stations) {}

    virtual ~TransportMode() = default; // Destructor virtual obligatorio para borrar "hijos de la memoria"

    const std::string& name() const { return name_; } //Metodos simples para consultar el nombre y las estaciones
    int stations() const { return stations_; }

    // 1.1: Métodos virtuales puros (clase abstracta) (los =0)
    virtual long long baseFare() const = 0; // el =0 obliga a cada hijo definir su tarifa y su tipo
    virtual std::string serviceType() const = 0;

    // 1.2: Hooks virtuales con comportamiento por defecto (true)
    virtual bool allowsStudentFlatFare() const { return true; } //Porque casi todos los transportes del metro permiten descuento estudiantil y transbordos
    virtual bool allowsIntegration() const { return true; }
    virtual bool opensIntegrationWindow() const { return true; }

    // TODO 1.3: Template Method (no virtual) que genera la descripción del medio
    std::string describe() const { // no es virtual porque el texto se arma igual para todos los medios
        return serviceType() + " " + name_ + " (" + std::to_string(stations_) +
               " stations) - full fare " + formatCOP(baseFare());
    }

private:
    std::string name_;  //Nombre de la linea como "A" o "T-A Ayacucho"
    int stations_; //La cantidad de estaciones
};

// 1.4: Las 5 clases de cada tipo (Tarifa base este 2026 es: 3820 COP) (Clases hijas)
// Metro, el tranvia y el metrocable todos valen lo mismo y heredan el constructor del padre
class MetroLine : public TransportMode { // El metro
public:
    using TransportMode::TransportMode; //Usa el construtor del padre
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Metro line"; }
};

class TramLine : public TransportMode { //Este es el tranvia
public:
    using TransportMode::TransportMode;
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Tram line"; }
};

class CableLine : public TransportMode { //Este es el Metrocable
public:
    using TransportMode::TransportMode;
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Cable line"; }
};

// Cable Turístico: hereda de CableLine y añade recargo (cumple Regla C sin repetir 3820)
class TouristCable : public CableLine {
public:
    TouristCable(std::string name, int stations, long long surcharge) 
        : CableLine(std::move(name), stations), surcharge_(surcharge) {}

    long long baseFare() const override {
        return CableLine::baseFare() + surcharge_; // Llamada cualificada al padre (3820 + recargo)
    }
//Aqui en esta parte pues no da descuento de estudiante, ni integra ni abre 90 mins de ventana
    std::string serviceType() const override { return "Tourist cable"; }
    bool allowsStudentFlatFare() const override { return false; }
    bool allowsIntegration() const override { return false; }
    bool opensIntegrationWindow() const override { return false; }

private:
    long long surcharge_; //El recargo adicional
};

// Ruta Alimentadora (Elbus) (Ruta integrada, tarifa 0) ( Esta cubierto x el viaje principal )
class FeederRoute : public TransportMode {  //Clase alimentadora 
public:
    using TransportMode::TransportMode; 
    long long baseFare() const override { return 0; }
    std::string serviceType() const override { return "Feeder route"; }
    bool opensIntegrationWindow() const override { return false; } // No abre ventana para que no empiece gratis
};

/* -------------------------------------------------------------------------
   Parte 2 la interfaz no-virtual y no todos pagan lo mismo cuando pasan la tarjeta
   ------------------------------------------------------------------------- */
class Passenger {
public: //Guarda el nimbre del perfil como "Civica frequent" o "Municipal student" por ej
    explicit Passenger(std::string name) : name_(std::move(name)) {}
    virtual ~Passenger() = default; //Destructor virtual para liberar

    const std::string& name() const { return name_; } //Aca da el nombre del perfil (lo retorna)

    //2.1: Interfaz NVI (Metodo publico no virtual para que el metro no quede debiendo plata (no sea negativo))
    long long fareOn(const TransportMode& mode) const { //Llama la red de transporte cuando valida un usuario, y hace el calculo
        return std::max(0LL, computeFare(mode));//El computeFare si es virtual, cada tipo de pasajero sobreescribe para los descuentos
    }

    virtual std::string profile() const = 0; //Metodo puro: obliga a cada clase de pasajero a decir su perfil

protected: //Metodo protegido virtual, comportamiento por defecto (la tarifa del medio)
    virtual long long computeFare(const TransportMode& mode) const {
        return mode.baseFare();
    }

private:
    std::string name_;
};

//2.2: Los 5 perfiles de pasajeros con sus respectivos descuentos/tarifas (Clases hijas de pasajeros)

class FrequentPassenger : public Passenger {
public:
    using Passenger::Passenger;
    std::string profile() const override { return "Civica frequent"; } //Usuario comun paga tarifa completa
};

class StudentPassenger : public Passenger {
public:
    using Passenger::Passenger;
    std::string profile() const override { return "Municipal student"; } //estudiante paga $1600 si se permite

protected:
    long long computeFare(const TransportMode& mode) const override {
        if (mode.allowsStudentFlatFare()) { //Pregunta si se permite tarifa de estudiante con la regla A de no usar Dynamic Cast ni comparar strings
            return 1600; // Tarifa plana preferencial
        }
        return Passenger::computeFare(mode); // Tarifa plena si el medio no lo permite como el cable arvi
    }
};

class SeniorPassenger : public Passenger { // Adulto mayor resta $490 pesos a la tarifa del padre
public:
    using Passenger::Passenger;
    std::string profile() const override { return "Senior citizen"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) - 490; // Descuento de adulto mayor
    }
};
//Persona con discapacidad resta $1100 a la tarifa del padre
class DisabilityPassenger : public Passenger {
public:
    using Passenger::Passenger;
    std::string profile() const override { return "Person with disability"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) - 1100; // Descuento por discapacidad
    }
};
//Pasaje al portador (ocasional) suma $580
class BearerPassenger : public Passenger {
public:
    using Passenger::Passenger;
    std::string profile() const override { return "Bearer (occasional)"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) + 580; // Recargo por pasaje al portador
    }
};

/* --------------------------------------------------------------
   PARTE 3- El itinerario y la red (Simula como cobra la civica)
   -------------------------------------------------------------- */
struct Leg {
    int minute;                   // minutos transcurridos desde el inicio del viaje
    const TransportMode* mode;    // puntero observador no propietario del medio
};

struct ChargedLeg {
    int minute;                   // minuto del cobro
    std::string modeName;          // nombre de la linea (ej. "A", "130 Robledo")
    std::string serviceType;       // tipo de servicio (ej. "Metro line")
    long long charge;              // valor cobrado
    bool integrated;              // true si fue trasbordo gratuito dentro de la ventana de 90 min
};

//3.1: Clase Network gestionando propiedad con std::unique_ptr
class Network {
public:
    TransportMode& add(std::unique_ptr<TransportMode> mode) { //La red pues network es la unica dueña y puntero
        modes_.push_back(std::move(mode)); // Guarda el medio en la red moviendo el puntero unico
        return *modes_.back(); // Retorna una referencia para usar en main sin quitar propiedad
    }

    const std::vector<std::unique_ptr<TransportMode>>& modes() const {
        return modes_; // Devuelve todos los medios guardados
    }

    void printInventory() const { // Recorre los medios e imprime su descripcion
        for (const auto& m : modes_) {
            std::cout << "  " << m->describe() << "\n";
        }
    }

private:
    std::vector<std::unique_ptr<TransportMode>> modes_; // Punteros inteligentes que borran la memoria solos
};

// 3.2: Aplicación de las reglas de integración y cobro de itinerario
std::vector<ChargedLeg> chargeItinerary(std::vector<Leg> legs,
                                        const Passenger& passenger) {
    // a) Ordenar cronológicamente por minuto con std::sort y una lambda
    std::sort(legs.begin(), legs.end(), [](const Leg& a, const Leg& b) {
        return a.minute < b.minute; // Ordena de menor a mayor minuto
    });

    std::vector<ChargedLeg> result; // Aqui guardamos los tramos cobrados
    bool windowOpen = false;        // Nos dice si hay una ventana de 90 min activa
    int windowStartMinute = 0;     // Minuto en que se abrio la ventana actual
    int freeTransfersUsed = 0;     // Contador de trasbordos gratis usados (max 2)

    // Único ciclo explícito permitido en todo el programa
    for (const auto& leg : legs) {
        const TransportMode& mode = *leg.mode; // Medio de transporte de este tramo
        long long charge = 0;                  // Valor a cobrar en este tramo
        bool isFreeTransfer = false;           // Bandera de trasbordo gratis

        if (!mode.allowsIntegration()) {
            // d) Medio sin integración: siempre paga y no modifica la ventana (ej. Arvi)
            charge = passenger.fareOn(mode);
        } else {
            // Revisa si estamos dentro del rango de 90 minutos desde que abrio la ventana
            bool inWindow = windowOpen && ((leg.minute - windowStartMinute) <= 90);

            if (inWindow && freeTransfersUsed < 2) {
                // c) Trasbordo gratuito dentro de la ventana de 90 min (hasta 2 gratis)
                charge = 0;
                isFreeTransfer = true;
                freeTransfersUsed++; // Suma uno a los trasbordos gratis gastados
            } else {
                // f) Ventana expirada o límite alcanzado: paga y abre nueva ventana
                charge = passenger.fareOn(mode);
                if (mode.opensIntegrationWindow()) {
                    windowOpen = true;             // Abre nueva ventana de trasbordo
                    windowStartMinute = leg.minute; // Guarda el minuto de inicio
                    freeTransfersUsed = 0;          // Reinicia los trasbordos gratis
                }
            }
        }

        // Guarda el resultado final de este tramo en la lista
        result.push_back({leg.minute, mode.name(), mode.serviceType(), charge, isFreeTransfer});
    }

    return result; // Devuelve el itinerario ya cobrado
}

/* -------------------------------------------------------------------------
   PARTE 4 Reportes con algoritmos STL y lambda
   ------------------------------------------------------------------------- */

// 4.1: Total pagado con std::accumulate (Sin ciclos explícitos)
long long totalPaid(const std::vector<ChargedLeg>& itinerary) {
    return std::accumulate(itinerary.begin(), itinerary.end(), 0LL,
        [](long long sum, const ChargedLeg& leg) { return sum + leg.charge; }); // Va sumando el cobro de cada tramo
}

//4.2: Cantidad de trasbordos gratuitos con std::count_if (Sin ciclos explícitos)
long long freeTransferCount(const std::vector<ChargedLeg>& itinerary) {
    return std::count_if(itinerary.begin(), itinerary.end(),
        [](const ChargedLeg& leg) { return leg.integrated; }); // Cuenta los tramos con trasbordo gratis
}

//4.3: Acumulado por tipo de servicio con std::for_each (Sin ciclos explícitos)
std::map<std::string, long long> paidByServiceType(const std::vector<ChargedLeg>& itinerary) {
    std::map<std::string, long long> totals; // Diccionario tipo servicio -> dinero cobrado
    std::for_each(itinerary.begin(), itinerary.end(),
        [&totals](const ChargedLeg& leg) { totals[leg.serviceType] += leg.charge; }); // Acumula por tipo
    return totals;
}

//4.4: Inyección de políticas con std::function y std::for_each (Sin ciclos explícitos)
std::vector<std::string> namesMatching(
    const Network& net,
    const std::function<bool(const TransportMode&)>& policy) {
    std::vector<std::string> result;
    std::for_each(net.modes().begin(), net.modes().end(),
        [&result, &policy](const std::unique_ptr<TransportMode>& m) {
            if (policy(*m)) { // Si cumple el criterio de la funcion inyectada
                result.push_back(m->name()); // Guarda el nombre del medio
            }
        });
    return result;
}

//4.5: Unir nombres con separador ", " usando std::accumulate (Sin ciclos explícitos)
std::string joined(const std::vector<std::string>& names) {
    if (names.empty()) return "";
    return std::accumulate(std::next(names.begin()), names.end(), names[0],
        [](const std::string& a, const std::string& b) { return a + ", " + b; }); // Pega palabras con comas
}

/* -------------------------------------------------------------------------
   MAIN (PRUEBA QUE NOS DIO EL PROFE SIN MODIFICARLA)
   ------------------------------------------------------------------------- */
int main() {
    std::cout << "============================================================\n";
    std::cout << "   INTEGRATED ITINERARY AND FARE ENGINE - Valle de Aburra\n";
    std::cout << "============================================================\n\n";

    Network network;
    // Registra cada medio de transporte en la red usando unique_ptr
    TransportMode& lineA  = network.add(std::make_unique<MetroLine>("A", 21));
    TransportMode& lineB  = network.add(std::make_unique<MetroLine>("B", 7));
    TransportMode& tramTA = network.add(std::make_unique<TramLine>("T-A Ayacucho", 9));
    TransportMode& cableK = network.add(std::make_unique<CableLine>("K Santo Domingo", 4));
    TransportMode& arvi   = network.add(std::make_unique<TouristCable>("L Arvi", 2, 9180));
    TransportMode& feeder = network.add(std::make_unique<FeederRoute>("130 Robledo", 18));

    std::cout << ">> 1. NETWORK INVENTORY (polymorphic describe)\n";
    network.printInventory();
    std::cout << '\n';

    /* Itinerario desordenado a proposito para verificar que el sort ordene por minuto */
    const std::vector<Leg> itinerary = {
        {45,  &cableK},
        {0,   &feeder},
        {12,  &lineA},
        {150, &lineB},
        {70,  &tramTA},
        {200, &arvi}
    };

    // Creacion de los 3 usuarios de prueba
    const FrequentPassenger frequent("Diego");
    const StudentPassenger  student("Valentina");
    const SeniorPassenger   senior("Dona Amparo");

    std::cout << ">> 2. FULL FARE PER PROFILE ON METRO LINE A\n";
    std::cout << "  " << frequent.profile() << " pays " << formatCOP(frequent.fareOn(lineA)) << '\n';
    std::cout << "  " << student.profile()  << " pays " << formatCOP(student.fareOn(lineA))  << '\n';
    std::cout << "  " << senior.profile()   << " pays " << formatCOP(senior.fareOn(lineA))   << '\n';
    std::cout << "  " << student.profile()  << " on the tourist cable pays "
              << formatCOP(student.fareOn(arvi)) << " (no flat fare there)\n\n";

    // Procesa el itinerario para el pasajero frecuente
    const std::vector<ChargedLeg> charged = chargeItinerary(itinerary, frequent);

    std::cout << ">> 3. CHARGED ITINERARY (frequent profile)\n";
    std::for_each(charged.begin(), charged.end(), [](const ChargedLeg& c) {
        std::cout << "  min " << c.minute << " | " << c.serviceType << " " << c.modeName
                  << " | " << formatCOP(c.charge)
                  << (c.integrated ? "  [free transfer]" : "") << '\n';
    });
    std::cout << '\n';

    // Imprime los reportes calculados con los algoritmos STL
    std::cout << ">> 4. REPORTS BUILT WITH STL ALGORITHMS\n";
    std::cout << "  accumulate: total paid -> " << formatCOP(totalPaid(charged)) << '\n';
    std::cout << "  count_if: free transfers -> " << freeTransferCount(charged) << '\n';

    const auto byType = paidByServiceType(charged);
    std::for_each(byType.begin(), byType.end(),
        [](const std::pair<const std::string, long long>& entry) {
            std::cout << "  map: paid on " << entry.first << " -> "
                      << formatCOP(entry.second) << '\n';
        });

    const auto firstPaid = std::find_if(charged.begin(), charged.end(),
        [](const ChargedLeg& c) { return c.charge > 0; });
    std::cout << "  find_if: first leg that actually paid -> "
              << (firstPaid == charged.end() ? std::string("none") : firstPaid->modeName) << '\n';

    // Inyeccion de politica: filtrar medios con mas de 8 estaciones
    const std::function<bool(const TransportMode&)> bigModes =
        [](const TransportMode& m) { return m.stations() > 8; };
    std::cout << "  injected policy: modes with more than 8 stations -> "
              << joined(namesMatching(network, bigModes)) << '\n';

    std::cout << "  total for the student on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, student))) << '\n';
    std::cout << "  total for the senior on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, senior))) << '\n';

    std::cout << "\n============================================================\n";
    std::cout << "  End of demonstration.\n";
    std::cout << "============================================================\n";
    return 0;
} //Gracias profe, un placer
