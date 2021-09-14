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

//! \brief lit le contenu du fichier dont le nom est passé en paramètre et insère chaque ligne dans un vecteur.
//! les doubles guillemet sont retirés de chaque ligne. Il est à noter que la première ligne du fichier est
//! ignorée puisqu'elle doit contientir les noms des champs dans le fichier CSV
//! \param[in] p_nomFichier: le nom du fichier à lire
//! \return le vecteur de string contenant toutes les lignes du fichier ayant été épurées du caractère \"
//! \exception logic_error si le fichier est introuvable ou ne peut pas être lu
std::vector<std::string> lireFichierCsv(const std::string &p_nomFichier)
{
    ifstream fichierLignes;
    fichierLignes.open(p_nomFichier);

    if (!fichierLignes) throw logic_error("Le fichier " + p_nomFichier + " n'existe pas.");

    const char CHARARETIRER {'\"'};
    string ligne {};
    std::vector<std::string> lignes;

    fichierLignes.ignore(numeric_limits<streamsize>::max(), '\n');
    while (getline(fichierLignes, ligne))
    {
        ligne.erase(remove_if(ligne.begin(), ligne.end(),
                              [&CHARARETIRER](const char &c) {
                                  return CHARARETIRER == c;
                              }), ligne.end());
        lignes.push_back(ligne);
    }

    fichierLignes.close();
    return lignes;
}

Date string_to_date(const std::string &strDate)
{
    const unsigned int lngChampAnnee = 4;
    const unsigned int lngChampMoisJour = 2;

    stringstream anneeSStream, moisSStream, jourSStream;
    unsigned int annee, mois, jour;

    anneeSStream << strDate.substr(0, lngChampAnnee);
    anneeSStream >> annee;

    moisSStream << strDate.substr(lngChampAnnee, lngChampMoisJour);
    moisSStream >> mois;

    jourSStream << strDate.substr(lngChampAnnee + lngChampMoisJour, lngChampMoisJour);
    jourSStream >> jour;

    return Date (annee, mois, jour);
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
    stringstream idRouteSStream;

    vector<string> champsDeLigne;

    for (const auto &strLigneDuFichier : lireFichierCsv(p_nomFichier))
    {
        champsDeLigne = string_to_vector(strLigneDuFichier, DELIMITEUR);

        stringstream().swap(idRouteSStream);
        idRouteSStream << champsDeLigne.at(0);
        idRouteSStream >> idRoute;
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
    double latitude, longitude;
    Station station;
    stringstream stationSStream;

    vector<string> champsDeLigne;

    for (const auto &strLigneDuFichier : lireFichierCsv(p_nomFichier))
    {
        champsDeLigne = string_to_vector(strLigneDuFichier, DELIMITEUR);

        stringstream().swap(stationSStream);
        stationSStream << champsDeLigne.at(0) << champsDeLigne.at(3) << champsDeLigne.at(4);
        stationSStream >> idStation >> latitude >> longitude;
        Coordonnees coords (latitude, longitude);

        station = Station(idStation, champsDeLigne.at(1), champsDeLigne.at(2), coords);
        m_stations.insert(pair<unsigned int, Station> (station.getId(), station));
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

}


//! \brief ajoute les services de la date du GTFS (m_date)
//! \param[in] p_nomFichier: le nom du fichier contenant les services
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterServices(const std::string &p_nomFichier)
{

//écrire votre code ici
    string idService, strDate;
    stringstream conversionSStream;
    unsigned int typeException;

    vector<string> champsDeLigne;

    for (const auto &strLigneDuFichier : lireFichierCsv(p_nomFichier))
    {
        champsDeLigne = string_to_vector(strLigneDuFichier, DELIMITEUR);

        idService = champsDeLigne.at(0);
        strDate = champsDeLigne.at(1);

        stringstream().swap(conversionSStream);
        conversionSStream << champsDeLigne.at(2);
        conversionSStream >> typeException;
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
    string idService, idVoyage, destination;
    stringstream conversionSStream;
    unsigned int idLigne;

    vector<string> champsDeLigne;

    for (const auto &strLigneDuFichier : lireFichierCsv(p_nomFichier))
    {
        champsDeLigne = string_to_vector(strLigneDuFichier, DELIMITEUR);

        idService = champsDeLigne.at(1);
        idVoyage = champsDeLigne.at(2);

        stringstream().swap(conversionSStream);
        conversionSStream << champsDeLigne.at(0) << champsDeLigne.at(3);
        conversionSStream >> idLigne >> destination;

        auto serviceCorrespondant = m_services.find(idService);
        if (serviceCorrespondant != m_services.end())
        {
            Voyage voyage (idVoyage, idLigne, idService, destination);
            m_voyages.insert(pair<std::string, Voyage> (voyage.getId(), voyage));
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

}



