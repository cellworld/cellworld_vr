### general multiplayer basics ue5.3.2

[video](https://www.youtube.com/watch?v=t0NYKaSh_hY)

- Need to use `AGameMode`, which was the first version of "gamemode" classes, `AGameModeBase` has no online functionality (i.e. use only for single player)

- `AGameInstance`: keeps track of connecting, disconnecting as well as stores values during entire process lifetime (never gets destroyed during game). 

- `AGameMode`: only accessible in the server 

- `AGameState` & `APlayerState`: created in the server and is replicated to clients 
 
- `Server & Clients` vs. `Server & Owning Clients`: 15:25

- `Owning Clients`: 