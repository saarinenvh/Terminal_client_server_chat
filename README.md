# Terminal Client Server chat

Ville Saarinen 
Joona Haavisto 

KeskusteluohjelmaOhjelman toteutuksen ja rakenteen kuvaus
Ohjelma koostuu palvelimesta (server.c) ja asiakasohjelmasta (client.c). Palvelin odottaa
yhteyksiä asiakasohjelmilta ja ylläpitää listaa yhteyksistä ja käyttäjänimistä
“user”-tietorakenteissa.

Kun asiakasohjelma ottaa yhteyden palvelimeen yhteys hyväksytään ja käydään
viestinvaihto, jossa varmistetaan että yhteys toimii (palvelin lähettää “CONNECTED” viestin
ja asiakasohjelma vastaa “ACKNOWLEDGED”). Sen jälkeen palvelin kysyy käyttäjän nimen
ja ilmoittaa muut kanavalla olevat käyttäjät. “Users”-taulukosta etsitään ensimmäinen vapaa
paikka, johon tallennetaan uusi pistoke sekä käyttäjänimi. Kaikille kanavalle olijoille
ilmoitetaan uuden käyttäjän liittyminen ja asiakasohjelmasta voi aloittaa viestien
lähettämisen palvelimelle, joka lähettää ne eteenpäin kaikille muille käyttäjille.

Käännös- ja käyttöohjeet

Ohjelman voi kääntää “make” komennolla. Pelkän palvelimen saa käännettyä komennolla
“make server” ja asiakasohjelman komennolla “make client”.
Palvelin käynnistetään ./server <kanavannimi> <portti>.
Asiakasohjelma käynnistetään kommennolla ./client <palvelimen osoite> <portti>. Yhteyden
voi katkaista lähettämällä viestin “disconnect”.

Käytetyn protokollan kuvaus
Palvelin ja asiakasohjelmat väliset viestit ovat normaalia tekstiä. Viestien mukana ei ole
muita tietoja. Yhteyttä muodostettaessa palvelin ja asiakasohjelma testaavat yhteyttä
“CONNECTED” - “ACKNOWLEDGED” -viestinvaihdolla.Testaus
Ohjelmaa on testattu peruskäytöllä eri tietokoneiden välillä. Virhetilanteita testattiin
sammuttamalla palvelin tai asiakasohjelma yllättäen.

Ohjelman toiminnan analysointi

Kuljetuskerroksen protokollan valinta
Valitsimme TCP:n, jotta avoimia yhteyksiä voi seurata palvelin ohjelmassa. TCP varmistaa
myös, ettei viestit katoa asiakasohjelmien ja palvelimen välillä.

IPv6 - osoitteiden tuki
Ohjelma ei tue IPv6 osoitteita.

Mitä tapahtuu kun asiakasohjelma sammuttaa äkisti koneensa kesken
yhteyden?

Kun palvelin huomaa yhteyden katkenneen, se vapauttaa “users”-taulukkoon paikan, jossa
kyseisen yhteyden tiedot olivat tallennettuna ja ilmoittaa muille käyttäjille käyttäjän
katkaisseen yhteyden.Mitä tapahtuu kun palvelin sammuttaa äkisti koneensa kesken
yhteyden?

Asiakasohjelma seuraa SIGPIPE-signaaleja, ja huomatessaan yhteyden katkenneen
ilmoittaa käyttäjälle ja sulkee ohjelman.

