# IoT
Mini Projet
Système de Contrôle d'Accès Connecté et Sécurisé (IoT ESP32 & Blynk)
 1. Contexte et Objectifs du Projet
Dans le cadre de la sécurisation des infrastructures modernes, la simple authentification par badge physique (comme le RFID classique) présente des vulnérabilités critiques (perte, vol ou duplication de badge). Ce projet propose une solution de contrôle d'accès intelligente et hautement sécurisée s'appuyant sur l'Internet des Objets (IoT).
 Objectifs Majeurs de Sécurité :
Authentification Multi-facteur (MFA / 2FA) : L'accès ne dépend pas uniquement du badge RFID. Une fois le badge légitime détecté, une double validation humaine est requise depuis l'application mobile Blynk sous un délai strict de 30 secondes.

Mécanisme Anti-Brute-Force : Protection logicielle active contre les attaques par dictionnaire ou essais successifs. Après 4 tentatives infructueuses, le système se verrouille totalement pendant 90 secondes et déclenche une alerte sonore (Buzzer) pour dissuader l'intrus.

Chiffrement des Communications (SSL/TLS) : Utilisation de la bibliothèque BlynkSimpleEsp32_SSL.h et du port sécurisé 443 pour chiffrer l'intégralité des flux de données entre l'ESP32 et le Cloud Blynk, bloquant les attaques de type Man-In-The-Middle (MITM).

Gestion Intelligente de l'Éveil : Détection de présence en amont via un capteur de mouvement (PIR) pour n'activer l'interface LCD et inviter au scan que lorsqu'un individu approche.
En combinant un microcontrôleur ESP32, une plateforme Cloud (Blynk) et une approche de sécurité multicouche, l'objectif est de concevoir un mécanisme de verrouillage robuste doté de protections avancées contre les intrusions physiques et numériques.

2. Architecture du Système
 Le système est structuré selon le modèle d'architecture IoT standardisé, organisé en couches successives pour assurer une séparation claire entre l'acquisition des données, le traitement local et la supervision distante :

Couche Utilisateur (Interface Graphique) :

Application Mobile / Web Blynk : Permet à l'utilisateur de recevoir des notifications d'alerte en temps réel, de valider le second facteur d'authentification (2FA) via un bouton virtuel dédié, ou de forcer l'ouverture manuelle de la porte.

Couche Cloud (Supervision & Routage) :

Serveur Blynk Cloud : Assure le pont de communication sécurisé (via protocole SSL/TLS sur le Port 443) entre l'application mobile et le microcontrôleur. Il gère l'historique des événements et l'envoi des notifications push.

Couche de Traitement Central (Logique Métier) :

Microcontrôleur ESP32 : Cerveau du système connecté au WiFi. Il héberge et exécute les algorithmes de sécurité (comparaison de l'UID RFID, gestion du compteur d'erreurs anti-brute-force, temporisation des 30 secondes du MFA, et commande des actionneurs).

Couche de Perception (Capteurs / Entrées) :

Capteur de mouvement PIR : Détecte une présence humaine à l'approche de la porte et réveille l'interface visuelle.

Module RFID-RC522 : Lit l'identifiant unique (UID) du badge de l'utilisateur et transmet l'information au processeur pour vérification.

Couche d'Action (Actionneurs / Sorties) :

Servo-moteur : Reçoit le signal PWM de l'ESP32 pour pivoter à 90° et simuler l'ouverture physique du pêne de la porte.

Écran LCD (via protocole I2C) : Affiche en temps réel l'état du système (Prêt, Accès Refusé, Système Verrouillé, Attente 2FA).

Buzzer Piézoélectrique : Émet un signal sonore strident en cas d'accès refusé ou de blocage de sécurité.

Diode LED Verte : Fournit un indicateur lumineux d'état (s'allume en continu lors de la détection de mouvement ou de l'ouverture).

3. Composants Utilisés et Câblage
Liste du Matériel :
Microcontrôleur ESP32 (NodeMCU-32S)

Capteur de mouvement PIR

Module RFID-RC522 (flux géré via l'application)

Écran LCD 1602 avec module I2C

Servo-moteur standard (Simulateur de porte)

Module Relais 5V

Buzzer piézoélectrique

LED Verte + Résistance (220 ohms)

4. Instructions d'Exécution
Étape 1 : Configuration de l'environnement de développement
Ouvrez l'IDE Arduino ou connectez-vous sur votre espace de simulation Wokwi.

Depuis le gestionnaire de bibliothèques (Library Manager), installez les dépendances requises :

Blynk (par Volodymyr Shymanskyy)

ESP32Servo (par Kevin Harrington)

LiquidCrystal_I2C (par Frank de Brabander)

Étape 2 : Configuration du Dashboard Blynk
Créez un compte sur Blynk.cloud.

Créez un nouveau modèle (Template) nommé ControleAccesRFID.

Configurez les Datastreams (Broches Virtuelles) suivants :

V0 (String) : Réception de l'UID de la carte RFID.

V1 (Integer) : Bouton d'ouverture manuelle (0 ou 1).

V2 (Integer) : Bouton de validation pour le second facteur 2FA (0 ou 1).

V4 (String) : Console / Affichage d'état du système.

Dans l'onglet Events, configurez un événement nommé rfid_scan pour l'envoi des notifications Push sur smartphone.
