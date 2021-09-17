//
// Created by Mario Marchand on 16-12-29.
//

#include "DonneesGTFS.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <limits>

using namespace std;

const char DELIMITEUR {','};

class LecteurFichierCsv
{
public:
    explicit LecteurFichierCsv(const string &p_nomFichier);
    ~LecteurFichierCsv();
    bool litProchaineLigne(string &ligne);

private:
    string epurerLigneDuFichier(string &ligneDuFichier);

    const char CHARARETIRER {'\"'};
    ifstream m_streamFichier;
};

LecteurFichierCsv::LecteurFichierCsv(const string &p_nomFichier)
{
    m_streamFichier.open(p_nomFichier);
    if (!m_streamFichier) throw logic_error("Le fichier " + p_nomFichier + " n'existe pas.");

    m_streamFichier.ignore(numeric_limits<streamsize>::max(), '\n');
}

LecteurFichierCsv::~LecteurFichierCsv()
{
    m_streamFichier.close();
}

//! \brief lit le contenu du fichier dont le nom est passé en paramètre et insère chaque ligne dans un vecteur.
//! les doubles guillemet sont retirés de chaque ligne. Il est à noter que la première ligne du fichier est
//! ignorée puisqu'elle doit contientir les noms des champs dans le fichier CSV
//! \param[in] p_nomFichier: le nom du fichier à lire
//! \return le vecteur de string contenant toutes les lignes du fichier ayant été épurées du caractère \"
//! \exception logic_error si le fichier est introuvable ou ne peut pas être lu
bool LecteurFichierCsv::litProchaineLigne(string &ligne)
{
    bool ligneEstLisible {true};
    if(!getline(m_streamFichier, ligne)) ligneEstLisible = false;

    ligne = epurerLigneDuFichier(ligne);
    return ligneEstLisible;
}

string LecteurFichierCsv::epurerLigneDuFichier(string &ligneDuFichier)
{
    ligneDuFichier.erase(remove_if(ligneDuFichier.begin(), ligneDuFichier.end(),
    [this](const char &c) {
        return CHARARETIRER == c;
    }), ligneDuFichier.end());

    return ligneDuFichier;
}

//! \brief convertit une chaîne de caractères correspondant à une date de format AAAAMMJJ en un objet Date
//! \param[in] p_strDate: la date en string de format AAAAMMJJ
//! \return la date correspondant au string passé en paramètre
Date string_to_date(const std::string &p_strDate)
{
    const unsigned int lngChampAnnee = 4;
    const unsigned int lngChampMoisJour = 2;

    unsigned int annee, mois, jour;

    annee = stoi(p_strDate.substr(0, lngChampAnnee));
    mois = stoi(p_strDate.substr(lngChampAnnee, lngChampMoisJour));
    jour = stoi(p_strDate.substr(lngChampAnnee + lngChampMoisJour, lngChampMoisJour));

    return Date (annee, mois, jour);
}

//! \brief convertit une chaîne de caractères correspondant à une heure de format HH:mm:SS en un objet Heure
//! \param[in] p_strHeure: l'heure en string de format HH:mm:SS
//! \return l'heure correspondant au string passé en paramètre
Heure string_to_heure(const std::string &p_strHeure)
{
    const unsigned int lngChamp = 2;
    const unsigned int prochainePosition = 3;

    unsigned int heures, minutes, secondes;

    heures = stoi(p_strHeure.substr(0, lngChamp));
    minutes = stoi(p_strHeure.substr(prochainePosition, lngChamp));
    secondes = stoi(p_strHeure.substr(prochainePosition * 2, lngChamp));

    return Heure (heures, minutes, secondes);
}

//! \brief ajoute les lignes dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les lignes
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterLignes(const std::string &p_nomFichier)
{

//écrire votre code ici
    unsigned int idRoute;
    Ligne ligne;
    CategorieBus categorie;

    string ligneDuFichier;
    vector<string> champsDeLigne;

    LecteurFichierCsv lecteur(p_nomFichier);
    while (lecteur.litProchaineLigne(ligneDuFichier))
    {
        champsDeLigne = string_to_vector(ligneDuFichier, DELIMITEUR);

        idRoute = stoi(champsDeLigne.at(0));
        categorie = Ligne::couleurToCategorie(champsDeLigne.at(7));

        ligne = Ligne(idRoute, champsDeLigne.at(2), champsDeLigne.at(4), categorie);
        m_lignes[ligne.getId()] = ligne;
        m_lignes_par_numero.insert(pair<string, Ligne> (ligne.getNumero(), ligne));
    }
}

//! \brief ajoute les stations dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterStations(const std::string &p_nomFichier)
{

//écrire votre code ici
    unsigned int idStation;
    double latitude;
    double longitude;
    Station station;

    string ligneDuFichier;
    vector<string> champsDeLigne;

    LecteurFichierCsv lecteur(p_nomFichier);
    while (lecteur.litProchaineLigne(ligneDuFichier))
    {
        champsDeLigne = string_to_vector(ligneDuFichier, DELIMITEUR);

        idStation = stoi(champsDeLigne.at(0));
        latitude = stod(champsDeLigne.at(3));
        longitude = stod(champsDeLigne.at(4));
        Coordonnees coords (latitude, longitude);

        station = Station(idStation, champsDeLigne.at(1), champsDeLigne.at(2), coords);
        m_stations.emplace(station.getId(), station);
    }
}

//! \brief ajoute les transferts dans l'objet GTFS
//! \breif Cette méthode doit âtre utilisée uniquement après que tous les arrêts ont été ajoutés
//! \brief les transferts (entre stations) ajoutés sont uniquement ceux pour lesquelles les stations sont prensentes dans l'objet GTFS
//! \brief les transferts sont ajoutés dans m_transferts
//! \brief les from_station_id des stations de transfert sont ajoutés dans m_stationsDeTransfert
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
//! \throws logic_error si tous les arrets de la date et de l'intervalle n'ont pas été ajoutés
void DonneesGTFS::ajouterTransferts(const std::string &p_nomFichier)
{

//écrire votre code ici
    if (!m_tousLesArretsPresents)
    {
        throw logic_error("Tous les arrêts de la date et de l'intervalle n'ont pas été ajoutés.");
    }

    unsigned int idStationDepart;
    unsigned int idStationArrivee;
    unsigned int tempsMinimal;

    string ligneDuFichier;
    vector<string> champsDeLigne;

    LecteurFichierCsv lecteur(p_nomFichier);
    while (lecteur.litProchaineLigne(ligneDuFichier))
    {
        champsDeLigne = string_to_vector(ligneDuFichier, DELIMITEUR);

        idStationDepart = stoi(champsDeLigne.at(0));
        idStationArrivee = stoi(champsDeLigne.at(1));
        tempsMinimal = stoi(champsDeLigne.at(3));
        tempsMinimal = tempsMinimal > 0 ? tempsMinimal : 1;

        bool transfertDoitEtreAjoute {false};
        if (idStationDepart == idStationArrivee)
        {
            if (m_stations.find(idStationDepart) != m_stations.end()) transfertDoitEtreAjoute = true;
        }
        else if (m_stations.find(idStationDepart) != m_stations.end() && m_stations.find(idStationArrivee) != m_stations.end())
        {
            transfertDoitEtreAjoute = true;
        }

        if (transfertDoitEtreAjoute)
        {
            m_transferts.emplace_back(idStationDepart, idStationArrivee, tempsMinimal);
            m_stationsDeTransfert.insert(idStationDepart);
        }
    }
}


//! \brief ajoute les services de la date du GTFS (m_date)
//! \param[in] p_nomFichier: le nom du fichier contenant les services
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterServices(const std::string &p_nomFichier)
{

//écrire votre code ici
    string idService;
    string strDate;
    unsigned int typeException;

    string ligneDuFichier;
    vector<string> champsDeLigne;

    LecteurFichierCsv lecteur(p_nomFichier);
    while (lecteur.litProchaineLigne(ligneDuFichier))
    {
        champsDeLigne = string_to_vector(ligneDuFichier, DELIMITEUR);

        idService = champsDeLigne.at(0);
        strDate = champsDeLigne.at(1);

        typeException = stoi(champsDeLigne.at(2));
        Date dateService = string_to_date(strDate);

        if (typeException == 1 && dateService == m_date)
        {
            m_services.insert(idService);
        }
    }
}

//! \brief ajoute les voyages de la date
//! \brief seuls les voyages dont le service est présent dans l'objet GTFS sont ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les voyages
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterVoyagesDeLaDate(const std::string &p_nomFichier)
{

//écrire votre code ici
    string idService;
    string idVoyage;
    string destination;
    unsigned int idLigne;

    string ligneDuFichier;
    vector<string> champsDeLigne;

    LecteurFichierCsv lecteur(p_nomFichier);
    while (lecteur.litProchaineLigne(ligneDuFichier))
    {
        champsDeLigne = string_to_vector(ligneDuFichier, DELIMITEUR);

        idService = champsDeLigne.at(1);
        idVoyage = champsDeLigne.at(2);
        idLigne = stoi(champsDeLigne.at(0));
        destination = champsDeLigne.at(3);

        if (m_services.find(idService) != m_services.end())
        {
            Voyage voyage (idVoyage, idLigne, idService, destination);
            m_voyages.emplace(voyage.getId(), voyage);
        }
    }
}

//! \brief ajoute les arrets aux voyages présents dans le GTFS si l'heure du voyage appartient à l'intervalle de temps du GTFS
//! \brief Un arrêt est ajouté SSI son heure de départ est >= now1 et que son heure d'arrivée est < now2
//! \brief De plus, on enlève les voyages qui n'ont pas d'arrêts dans l'intervalle de temps du GTFS
//! \brief De plus, on enlève les stations qui n'ont pas d'arrets dans l'intervalle de temps du GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les arrets
//! \post assigne m_tousLesArretsPresents à true
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterArretsDesVoyagesDeLaDate(const std::string &p_nomFichier)
{

//écrire votre code ici
    string idVoyage;
    string strHeureArrive;
    string strHeureDepart;
    unsigned int idStation;
    unsigned int numeroSequence;

    string ligneDuFichier;
    vector<string> champsDeLigne;

    LecteurFichierCsv lecteur(p_nomFichier);
    while (lecteur.litProchaineLigne(ligneDuFichier))
    {
        champsDeLigne = string_to_vector(ligneDuFichier, DELIMITEUR);

        idVoyage = champsDeLigne.at(0);

        // L'arrêt doit faire partie d'un voyage de la date
        if (m_voyages.find(idVoyage) != m_voyages.end())
        {
            strHeureArrive = champsDeLigne.at(1);
            strHeureDepart = champsDeLigne.at(2);
            Heure heureArrivee = string_to_heure(strHeureArrive);
            Heure heureDepart = string_to_heure(strHeureDepart);

            // L'arrêt doit être contenu dans l'intervale d'heure spécifiée
            if (heureArrivee < m_now2 && heureDepart >= m_now1)
            {
                idStation = stoi(champsDeLigne.at(3));
                numeroSequence = stoi(champsDeLigne.at(4));

                Arret::Ptr ptrArret = make_shared<Arret>(idStation, heureArrivee, heureDepart, numeroSequence, idVoyage);
                m_voyages[idVoyage].ajouterArret(ptrArret);
                m_stations[idStation].addArret(ptrArret);

                m_nbArrets++;
            }
        }
    }

    // Les voyages ne possédant aucun arrêt entre les heures spécifiées sont retirés
    for (auto it = m_voyages.cbegin(), itSuivant = it; it != m_voyages.cend(); it = itSuivant)
    {
        ++itSuivant;
        if (it->second.getNbArrets() == 0) m_voyages.erase(it);
    }

    // Les stations ne possédant aucun arrêt sont retirées
    for (auto it = m_stations.cbegin(), itSuivant = it; it != m_stations.cend(); it = itSuivant)
    {
        ++itSuivant;
        if (it->second.getNbArrets() == 0) m_stations.erase(it);
    }

    m_tousLesArretsPresents = true;
}



