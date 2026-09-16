#pragma GCC optimize("-Os")

#include <iostream>
#include <raylib.h> 
#include <deque>
#include <cstdlib>
#include <string>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

int cellCount = 30; // 10

int FoodNum = 5;
int BombNum = 5;

struct game_setting{
    public:

    // int cellSize = 30;
    // int grid_size = 15; // 10

    // int food_num = 5;
    // int bomb_num = 5;
    
    void LoadSettings(){
        std::ifstream f("game_settings.json");
        json data = json::parse(f);
        cellCount = (int)data["grid_size"];
        FoodNum = (int)data["food_num"];
        BombNum = (int)data["bomb_num"];
        highScore = (int)data["highScore"];
        time_interval = (double)data["time_interval"];
        walls = (bool)data["enable_walls"];
        sounds = (bool)data["enable_sounds"];
    }
    double GetTime(){
        return time_interval;
    }
    int GetScore() {
        return highScore;
    }
    bool GetWallState(){
        return walls;
    }
    bool GetSoundState(){
        return sounds;
    }
    void ChangeScore(int current){
        highScore = current;
        json data;
        // will it work if i not give rest of parametrs?
        data["grid_size"] = cellCount;
        data["food_num"] = FoodNum;
        data["bomb_num"] = BombNum;
        data["highScore"] = current;
        data["time_interval"] = time_interval;
        data["enable_walls"] = walls;
        data["enable_sounds"] = sounds;

        std::ofstream("game_settings.json") << data.dump(4);
    }
    void ToggleSound(){
        sounds = !sounds;
    }

    private:
    int highScore;
    double time_interval;
    bool walls;
    bool sounds;
} setting;


Color green = {173, 204, 96, 255};
Color dark_green = {43, 51, 24, 255};

const int cellSize = 30;

uint8_t grid[29][29]; // just do max grid even if not used

const int offset = 75;

bool allowMove = false;

int score = 0;

double lastTime = 0;


bool EventTrigger(double interval){
    double current = GetTime();
    if(current - lastTime >= interval){
        lastTime = current;
        return true;
    }
    return false;
}

bool ElementInDeque(Vector2 position, const std::deque<Vector2> &snake_body){
    for(auto &e : snake_body){
        if(e.x == position.x && e.y == position.y) return true;
    }
    return false;
}

class Food{
    public:
    Vector2 pos;
    // Texture2D texture;

    // Food(std::deque<Vector2> &snake_body) {
    //     pos = GetPosition(snake_body);
    // }
    Food() : pos{0, 0} {}

    ~Food(){
        // UnloadTexture(texture);
        // grid[(int)pos.y][(int)pos.x] = 0;
    }

    Vector2 GetPosition(std::deque<Vector2> &snake_body){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        while(ElementInDeque(Vector2{x, y}, snake_body) || (grid[(int)y][(int)x] == 1 || grid[(int)y][(int)x] == -1)){
            x = GetRandomValue(0, cellCount - 1);
            y = GetRandomValue(0, cellCount - 1);
        }
        grid[(int)y][(int)x] = 1;
        return Vector2{x, y};
    }

    void Draw(Texture2D &texture){ // get texture from game object
        DrawTexture(texture, offset + pos.x * cellSize, offset + pos.y * cellSize, WHITE);
    }
};

class Bomb{
    public:
    Vector2 pos;

    // Bomb(std::deque<Vector2> &snake_body) {
    //     pos = GetPosition(snake_body);
    // }

    Bomb() : pos{0, 0} {}

    ~Bomb(){
        // grid[(int)pos.y][(int)pos.x] = 0;
    }

    Vector2 GetPosition(std::deque<Vector2> &snake_body){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        while(ElementInDeque(Vector2{x, y}, snake_body) || (grid[(int)y][(int)x] != 0)){
            x = GetRandomValue(0, cellCount - 1);
            y = GetRandomValue(0, cellCount - 1);
        }
        grid[(int)y][(int)x] = -1;
        return Vector2{x, y};
    }

    void Draw(Texture2D &texture){ // get texture from game object
        DrawTexture(texture, offset + pos.x * cellSize, offset + pos.y * cellSize, WHITE);
    }
};

class Snake{
    public:
    std::deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;
    bool delSeg = false;

    void Draw(){
        for(int i = 0; i < (int)body.size(); i++){
            float x = body[i].x;
            float y = body[i].y;
            grid[(int)y][(int)x] = 2;
            // DrawRectangle(body[i].x * cellSize, body[i].y * cellSize, cellSize, cellSize, dark_green);
            DrawRectangleRounded(Rectangle{offset + x * cellSize, offset + y * cellSize, cellSize, cellSize}, 0.5, 6, dark_green);
        }
    }

    void Update(Sound effect){
        float cur_x = int(body[0].x + direction.x);
        float cur_y = int(body[0].y + direction.y);

        // if walls disabled then teleports to other side
        if(!setting.GetWallState()){
            if(cur_x < 0) {
                cur_x = cellCount - 1;
                if(setting.GetSoundState()) PlaySound(effect);
            }
            if(cur_y < 0) {
                cur_y = cellCount - 1;
                if(setting.GetSoundState()) PlaySound(effect);
            }
            if(cur_x >= cellCount) {
                cur_x = 0;
                if(setting.GetSoundState()) PlaySound(effect);
            }
            if(cur_y >= cellCount) {
                cur_y = 0;
                if(setting.GetSoundState()) PlaySound(effect);
            }
        }

        body.push_front(Vector2{cur_x, cur_y});
        // grid[(int)cur_y][(int)cur_x] = 1;

        if(addSegment){
            addSegment = false;
        }
        else{
            int lasty = body.back().y, lastx = body.back().x;
            grid[lasty][lastx] = 0;
            body.pop_back();
            if(delSeg){
                delSeg = false;
                body.pop_back();
                lasty = body.back().y, lastx = body.back().x;
                grid[lasty][lastx] = 0;
            }
        }
    }

    void Reset(){
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}}; // maybe randomize
        grid[9][6] = 2, grid[9][5] = 2, grid[9][4] = 2;
        direction = {1, 0};
    }
};

class Game{
    public:
    bool running = false;

    Texture2D texture_food;
    Texture2D texture_bomb;

    Snake snake;

    std::vector<Food> foods;
    std::vector<Bomb> bombs;

    Sound eatSound;
    Sound hitSound;
    Sound teleport;

    Game(){

        Image image = LoadImage("Graphics/food.png");
        texture_food = LoadTextureFromImage(image);

        Image image1 = LoadImage("Graphics/bad.png");
        texture_bomb = LoadTextureFromImage(image1);

        UnloadImage(image);
        UnloadImage(image1);

        if(setting.GetSoundState())
        {   
            InitAudioDevice();  
            eatSound = LoadSound("SoundEffects/eating.mp3");
            hitSound = LoadSound("SoundEffects/hit.mp3");
            teleport = LoadSound("SoundEffects/enderman.mp3");
        }

        for(int i = 0; i < FoodNum; i++){
            // foods[i] = Food(snake.body);
            foods.push_back(Food()); // optimize since body is fixed at the start
        }

        for(int i = 0; i < BombNum; i++){
            bombs.push_back(Bomb()); // i think i can optimize here
        } 

        ResetEntities();
    }

    ~Game(){
        if(setting.GetSoundState()){
            UnloadSound(eatSound);
            UnloadSound(hitSound);
            UnloadSound(teleport);
            CloseAudioDevice();
        }   
        UnloadTexture(texture_food);
        UnloadTexture(texture_bomb);
    }

    void ResetEntities(){
        for (int i = 0; i < FoodNum; i++) {
            foods[i].pos = foods[i].GetPosition(snake.body);
        }
        for (int i = 0; i < BombNum; i++) {
            bombs[i].pos = bombs[i].GetPosition(snake.body);
        }
    }

    void Update(){
        if(!running) return;
        if(setting.GetScore() < score){
            setting.ChangeScore(score);
            // setting.updateScore(score);
            // high_score = score;
            // SaveHighScore(high_score);
        }

        
        if(setting.GetWallState()) CheckCollisionWalls();
        CheckCollisionAllFoods();
        CheckCollisionAllBombs();

        snake.Update(teleport);

        CheckCollisionTail();
    }

    void Draw(){ // modify to make it with several objects
        // if(!running) return; 
        
        for(int i = 0; i < FoodNum; i++){
            foods[i].Draw(texture_food);
        }

        for(int i = 0; i < BombNum; i++){
            bombs[i].Draw(texture_bomb);
        }
    
        if(setting.GetWallState()) CheckCollisionWalls(); // idk maybe call me solved

        snake.Draw();
    }

    void CheckCollisionAllBombs(){ // Fuse this one with Food one by passing parametr, toggle state
        for(int i = 0; i < BombNum; i++){
            CheckCollisionBomb(i);
        }
    }

    void CheckCollisionBomb(int num = 0){
        if(bombs[num].pos.x == snake.body[0].x && bombs[num].pos.y == snake.body[0].y){
            // std::cout << "Eating food" << std::endl;
            score --;
            snake.delSeg = true;

            int first = bombs[num].pos.x, second = bombs[num].pos.y;
            grid[second][first] = 0;

            bombs[num].pos = bombs[num].GetPosition(snake.body);
            if(!setting.GetSoundState()) return;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionAllFoods(){ // modify to make it with several objects
        for(int i = 0; i < FoodNum; i++){
            CheckCollisionFood(i);
        }
    }

    void CheckCollisionFood(int num = 0){
        if(foods[num].pos.x == snake.body[0].x && foods[num].pos.y == snake.body[0].y){
            // std::cout << "Eating food" << std::endl;
            score ++;
            snake.addSegment = true;

            int first = foods[num].pos.x, second = foods[num].pos.y;
            grid[second][first] = 0;

            foods[num].pos = foods[num].GetPosition(snake.body);
            if(!setting.GetSoundState()) return;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionTail(){
        std::deque<Vector2> headless_body = snake.body;
        headless_body.pop_front();
        if(ElementInDeque(snake.body[0], headless_body)){
            // std::cout << "Game over" << std::endl;
            GameOver();
        }
    }

    void CheckCollisionWalls(){
        if((snake.body[0].x >= cellCount || snake.body[0].x < 0) || (snake.body[0].y < 0 || snake.body[0].y >= cellCount)){
            // running = false;
            GameOver();
        }
        // i updadte not there not in order
    }

    void GameOver()
    {   
        // SaveHighScore(high_score);
        if(setting.GetScore() < score){
            setting.ChangeScore(score);
        }

        snake.Reset();

        // memset reset of whole grid
        memset(grid, 0, sizeof(grid));

        ResetEntities();

        // for(int i = 0; i < FoodNum; i++){
        //     // grid[(int)foods[i].pos.y][(int)foods[i].pos.x] = 0;
        //     foods[i].pos = foods[i].GetPosition(snake.body);
        // }

        // for(int i = 0; i < BombNum; i++){
        //     // grid[(int)foods[i].pos.y][(int)foods[i].pos.x] = 0;
        //     bombs[i].pos = bombs[i].GetPosition(snake.body);
        // }

        running = false;
        score = 0;
        if(!setting.GetSoundState()) return;
        PlaySound(hitSound);
    }
};


int main() 
{

    setting.LoadSettings();

    InitWindow(2 * offset + cellCount * cellSize, 2 * offset + cellCount * cellSize, "Snake!");
    SetTargetFPS(60);

    Game game;

    // Image bimage = LoadImage("Graphics/bomb.png");
    // Texture2D btexture = LoadTextureFromImage(bimage);
    // UnloadImage(bimage);
    // DrawTexture(btexture, 0, 0, WHITE);
    
    // std::cout << "Game version" << RAYLIB_VERSION << std::endl; 
    // Image imageQ = LoadImage("Graphics/question.png");

    // Texture2D question = LoadTextureFromImage(imageQ);

    // UnloadImage(imageQ);

    while (!WindowShouldClose())
    {
    // Rendering
        BeginDrawing();
        // if(setting.GetSoundState() && GetMasterVolume() != 1.0f) SetMasterVolume(1.0f);
        
        // if(IsKeyDown(KEY_M)){ // interval add or when it is released
        //     // mute the sound
        //     SetMasterVolume(0.0f);
        //     setting.ToggleSound();
        //     // game.Update();
        // }

        // need button to update settings but only sound or time interval and like in the beginning to pause before action 
        // and pause the game and mute the game

        if(EventTrigger(setting.GetTime())){ //0.2
            game.Update();
            allowMove = true;
        }
        if(IsKeyDown(KEY_UP) && game.snake.direction.y != 1 && allowMove){
            game.snake.direction = Vector2{0, -1};
            game.running = true;
            allowMove = false;
        }
        else if(IsKeyDown(KEY_DOWN) && game.snake.direction.y != -1 && allowMove){
            game.snake.direction = Vector2{0, 1};
            game.running = true;
            allowMove = false;
        }
        else if(IsKeyDown(KEY_LEFT) && game.snake.direction.x != 1 && allowMove){
           game.snake.direction = Vector2{-1, 0};
           game.running = true;
           allowMove = false;
        }
        else if(IsKeyDown(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove){
            game.snake.direction = Vector2{1, 0};
            game.running = true;
            allowMove = false;
        }
        
        ClearBackground(green);
        
        DrawRectangleLinesEx(Rectangle{float(offset - 5), (float)offset - 5, (float)cellCount * cellSize + 10, (float)cellCount * cellSize + 10}, 5, dark_green);
        
        game.Draw();

        // DrawTexture(question, offset + cellSize, offset + cellSize, WHITE);

        DrawText(TextFormat("Retro Snake Score: %i", score), offset - 5, 20, 40, dark_green);
        DrawText(TextFormat("High Score: %i", setting.GetScore()), offset - 5, cellCount * cellSize + 90, 40, dark_green);
        
        EndDrawing();
    }
    
    CloseWindow();
}