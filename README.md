# PRSidentCardGame
Multiplayer cardgame 


To install the game for the first time, run the following commands into your terminal:  

    touch /memCardsPile  
    
    touch /memLobby  
    
    make


Rules of the PRSident (in french):

    Le premier joueur à s'inscrire est le premier à jouer. 
    Une fois tout les joueurs sont inscrits, le premier joueur reçoit sa pile de carte distribuée par le serveur.
    Il peut jouer la carte qu'il veut. 
    Les joueurs suivants ne peuvent jouer qu'une carte de valeur supérieure.
    La valeurs des cartes est la suivante:
        3 < 4 < 5 < 6 < 7 < 8 < 9 < 10 < Jack < Queen < King < As < 2 
    Si un joueur n'a pas de cartes de valeur plus élevée que la carte précédente dans la pile il peut
    passer son tour en entrant "100".
    Le 2 étant la valeur la plus élévée si elle est jouée par un joueur, les autres joueurs n'ont pas 
    d'autres choix que de skip.
    Le gagnant de la partie est le joueur qui arrive à finir ses cartes en premier.

Remarks : 
It's a multiplayer game, by default the maximum of players has been set to 2 (to make the testing easier).
If you fancy to play with more players just change the MAXPLAYER variable in the client.h and server.h files.
