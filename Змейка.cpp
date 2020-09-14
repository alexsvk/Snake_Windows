// Змейка.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <deque>
#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>
#include <sstream>
#include <string>
#include <windows.h>
#include <ctime>
#include <conio.h>

namespace pingle
{
    HANDLE std_output_handle;

    struct coord
    {
        coord decrease(int x, int y)
        {
            coord copy = *this;
            this->x -= x;
            this->y -= y;
            return copy;
        }

        bool operator==(const coord& c) const
        {
            return x == c.x && y == c.y;
        }

        bool operator!=(const coord& c) const
        {
            return x != c.x && y != c.y;
        }

        class MyHashFunction 
        {
        public:

            // Use sum of lengths of first and last names 
            // as hash function. 
            size_t operator()(const coord& p) const
            {
                std::stringstream ss;
                ss << p.x << p.y;
                size_t d;
                ss >> d;
                return d;
            }
        };

        SHORT x;
        SHORT y;
    };

    enum class direction { default_, left, up, right, down };
    typedef std::vector<coord> coord_container;
    typedef std::deque<direction> direction_container;

    class snake
    {
    public:
        snake(unsigned short length, coord head, direction d, 
            unsigned int height, unsigned int width, unsigned short max_length = 15U) :
            length(length), max_length(max_length), direction(d), 
            field_width(width), field_height(height), is_collision_(false)
        {
            for (int j = 0; j < length; ++j)
            {
                coordinates.push_front(head.decrease(0, 1));
            }
        }

        const direction& get_direction() const
        {
            return direction;
        }
        coord& get_head()
        {
            return coordinates.back();
        }
        const std::deque<coord>& get_coordinates() const
        {
            return coordinates;
        }

        bool is_collision() const
        {
            return is_collision_;
        }

        void push_back(const coord& c)
        {
            if (length + 1 == max_length)
            {
                return;
            }
            
            this->coordinates.push_front(c);
            length += 1;
        }

        void move(direction vector = direction::default_)
        {
            if (vector != direction::default_)
            {
                direction = vector;
            }
            
            coord head_shifted = get_head();
            switch (direction)                
            {
            case direction::left:
                head_shifted.x -= 1U;
                break;
            case direction::down:
                head_shifted.y += 1U;
                break;
            case direction::right:
                head_shifted.x += 1U;
                break;
            case direction::up:
                head_shifted.y -= 1U;
                break;
            }

            // Змейка cъела себя?
            auto end = std::next(coordinates.begin(), length - 1);
            if (std::find_if(coordinates.begin(), end, [head_shifted](const coord& coordinate)
                {
                    return coordinate.x == head_shifted.x && coordinate.y == head_shifted.y;
                }) != end)
            {
                is_collision_ = true;
                return;
            }

            // Очищаем позицию хвоста змейки...
            coordinates.pop_front();

            // Ставим голову змейки на новое место.
            coordinates.push_back(head_shifted);
        }

        void reset()
        {
            is_collision_ = false;
            coordinates.clear();
            length = 1;
            coord head = { static_cast<SHORT>(field_width / 2), static_cast<SHORT>(field_height / 2) };
            for (int j = 0; j < length; ++j)
            {
                coordinates.push_front(head.decrease(0, 1));
            }
        }

    private:
        unsigned short length;
        const unsigned short max_length;
        std::deque<coord> coordinates;
        direction direction;
        unsigned int field_width;
        unsigned int field_height;
        bool is_collision_;
    };

    struct menu
    {
        enum class choice { play, exit, demo };

        choice run(SHORT w, SHORT h)
        {
            SetConsoleCursorPosition(pingle::std_output_handle, { w / 2 - 3, h / 2 - 2 });
            puts("1-start\n");
            //SetConsoleCursorPosition(pingle::std_output_handle, { w / 2 - 3, h / 2 - 1 });
            //puts("2-demo\n");
            SetConsoleCursorPosition(pingle::std_output_handle, { w / 2 - 3, h / 2 });
            puts("Esc-exit\n");
            int symbol = 0;
            while (true)
            {
                symbol = _getch();
                switch (symbol)
                {
                case 49:
                    return choice::play;
                case 50:
                    return choice::demo;
                case 27:
                    return choice::exit;
                }
            }
        }
        
    };

    coord any_neighbour(SHORT i, SHORT j, SHORT w, SHORT h)
    {
        if (0 == j)
        {
            // справа, снизу, и сверху
            if (i > 0 && i < h - 1)
            {
                return { j + 1, i };
            }
            // справа и сверху
            if (i == 0)
            {
                return { j + 1, i };
            }
            // сверху и справа
            if (i == h - 1)
            {
                return { j, i - 1 };
            }
        }
        else if (j < w - 1) // между левым и правым столбцами
        {
            if (i > 0 && i < h - 1) // со всех сторон
            {
                return { j - 1, i };
            }
            if (i == 0) // слева, справа, и под позиц-й
            {
                return { j - 1, i };
            }
            if (i == h - 1) // слева, справа, и над позиц-й
            {
                return { j - 1, i };
            }
        }
        else if (j == w - 1)
        {
            if (i > 0 && i < h - 1) // сверху, снизу, и слева
            {
                return { j, i + 1 };
            }
            if (i == 0) // снизу и слева
            {
                return { j, i + 1 };
            }
            if (i == h - 1) // сверху и слева
            {
                return { j - 1, i };
            }
        }
    }

    class dfs
    {
    public:

        dfs(const coord& apple_coordinate, const coord& head, int w, int h) : 
            apple_coordinate(apple_coordinate),
            snake_head_coordinate(head)
        {
            for (SHORT i = 0; i < h; ++i)
            {
                for (SHORT j = 0; j < w; ++j)
                {
                    coord_container v;
                    if (0 == j)
                    {
                        // справа, снизу, и сверху
                        if (i > 0 && i < h - 1)
                        {
                            v.push_back({ j + 1, i });
                            v.push_back({ j, i + 1 });
                            v.push_back({ j, i - 1 });
                        }
                        // справа и снизу
                        if (i == 0) 
                        {
                            v.push_back({ j + 1, i });
                            v.push_back({ j, i + 1 });
                        }
                        // сверху и справа
                        if (i == h - 1)
                        {
                            v.push_back({ j, i - 1 });
                            v.push_back({ j + 1, i });
                        }
                    }
                    else if (j < w - 1) // между левым и правым столбцами
                    {
                        if (i > 0 && i < h - 1) // со всех сторон
                        {                           
                            v.push_back({ j - 1, i });
                            v.push_back({ j + 1, i });
                            v.push_back({ j, i + 1 });
                            v.push_back({ j, i - 1 });
                        }
                        if (i == 0) // слева, справа, и под позиц-й
                        {
                            v.push_back({ j - 1, i });
                            v.push_back({ j + 1, i });
                            v.push_back({ j, i + 1 });
                        }
                        if (i == h - 1) // слева, справа, и над позиц-й
                        {
                            v.push_back({ j - 1, i });
                            v.push_back({ j + 1, i });
                            v.push_back({ j, i - 1 });
                        }
                    }
                    else if (j == w - 1)
                    {
                        if (i > 0 && i < h - 1) // сверху, снизу, и слева
                        {
                            v.push_back({ j, i + 1 });
                            v.push_back({ j, i - 1 });
                            v.push_back({ j - 1, i});
                        }
                        if (i == 0) // снизу и слева
                        {
                            v.push_back({ j, i + 1 });
                            v.push_back({ j - 1, i });
                        }
                        if (i == h - 1) // сверху и слева
                        {
                            v.push_back({ j - 1, i });
                            v.push_back({ j, i - 1  });
                        }
                    }
                    edges[{j, i}] = v;
                }
            }
        }

        void set_apple_coordinate(const coord&& apple_coordinate)
        {
            this->apple_coordinate = apple_coordinate;
        }

        void set_snake_head_coordinate(const coord& snake_head_coordinate)
        {
            this->snake_head_coordinate = snake_head_coordinate;
        }

        void get_path(direction_container& shortest_path) const
        {
            for (auto coordinate = apple_coordinate; coordinate != snake_head_coordinate; coordinate = prior.at(coordinate))
            {
                auto& previous_coordinate = prior.at(coordinate);
                auto x_difference = coordinate.x - previous_coordinate.x;
                if (x_difference != 0)
                {
                    shortest_path.push_back(x_difference > 0 ? direction::right : direction::left);
                }
                else
                {
                    auto y_difference = coordinate.y - previous_coordinate.y;
                    shortest_path.push_back(y_difference > 0 ? direction::down : direction::up);
                }                
            }
            std::reverse(shortest_path.begin(), shortest_path.end());
        }

        bool DFS(coord v, coord from)
        {
            if (from.x == -1 && from.y == -1)
                goto f;
            if (marked[v] != 0)  // Если мы здесь уже были, то тут больше делать нечего
            {
                return false;
            }
            marked[v] = 1;   // Помечаем, что мы здесь были
            prior[v] = from;  // Запоминаем, откуда пришли
            if (v == snake_head_coordinate)   // Проверяем, конец ли
            {
               // std::cout << "Hooray! The path was found!\n";
                return true;
            }
           f: for (int i = 0; i < (int)edges[v].size(); ++i)  // Для каждого ребра
            {
                if (DFS( v, edges[v][i])) break;  // Запускаемся из соседа
            }
        }

        void clear()
        {
            marked.clear();
            prior.clear();
        }
    private:
        coord apple_coordinate;
        coord snake_head_coordinate;
        std::unordered_map<coord,coord_container, coord::MyHashFunction> edges;
        std::unordered_map<coord, int, coord::MyHashFunction> marked; // что уже посетили
        std::unordered_map<coord, coord, coord::MyHashFunction> prior; // откуда мы пришли в данную позицию
    };

    struct window
    {
        window(unsigned int width = 50U, unsigned int height = 30U) :
            sleep_time(100),
            width(width),
            height(height),
            snake_(1, { static_cast<SHORT>(width / 2), static_cast<SHORT>(height / 2) }, direction::right, width, height),
            is_demo(false),
            score(0)
        {
            MoveWindow(GetConsoleWindow(), 50, 50, 1000, 1000, true); // установка стартовой позиции окна консоли (50 и 50 - это пиксели
            srand(time(0)); // запуск генератора случайных чисел          
            generate_apple_pos();
        }

        void print_wall()
        {
            
            for (int y = 0; y < height; y++) // стандартный двойной цикл на отрисовку рамки
            {
                for (int x = 0; x < width; x++)
                {
                    char s;
                    if (x == 0 && y == 0) // в верхнем левом углу поля - символ соответствующего угла
                        s = static_cast<char>(218);
                    else if (x == 0 && y == height - 1) // нижний левый угол
                        s = static_cast <char>(192);
                    else if (y == 0 && x == width - 1) // верхний правый угол
                        s = static_cast <char>(191);
                    else if (y == height - 1 && x == width - 1) // нижний правый угол
                        s = static_cast <char>(217);
                    else if (y == 0 || y == height - 1) // верхняя и нижняя граница поля
                        s = static_cast <char>(196);
                    else if (x == 0 || x == width - 1) // левая и правая граница поля
                        s = static_cast <char>(179);
                    else s = ' '; // во всех остальных случаях должен быть просто пробел (означает пустую область поля)
                    putchar(s); // выводим символ
                }
                std::cout << std::endl;
            }
        }

        void show()
        {
            print_wall();
            menu::choice choice = menu_.run(width, height);
            switch (choice)
            {
            case menu::choice::play:
                system("cls");
                on_start();
                break;
            case menu::choice::demo:
                system("cls");
                is_demo = true;
                on_start();
                break;
            case menu::choice::exit:
                //on_exit();
                break;
            }
        }

        void generate_apple_pos()
        {
            const auto& snake_coords = this->snake_.get_coordinates();
            do
            {
                apple_x = rand() % (width - 2) + 1;
                apple_y = rand() % (height - 2) + 1;
            } while (std::find_if(snake_coords.begin(), snake_coords.end(), [=](const coord& coordinate)
                {
                    return coordinate.x == apple_x && coordinate.y == apple_y;
                }) != snake_coords.end());
        }

        void on_start()
        {
        restart:
            system("cls");
            score = 0;
            snake_.reset();
            print_wall();
            redraw_apple();
            print_score();
            bool lost = false, escape_key = false, first_iteration = true;
            
            dfs dfs_for_snake({ apple_x, apple_y }, snake_.get_head(), width, height);
            do
            {
                Sleep(sleep_time); // задержка
                
                coord tail_before_move = snake_.get_coordinates().front();
                
                // сдвигаем "змейку" логически
                if (is_demo)
                {
                    static direction_container spath;
                    if (spath.empty())// генерируем короткий путь от головы до "яблока"
                    {
                        if (false == first_iteration)
                        {
                            dfs_for_snake.clear();
                            dfs_for_snake.set_apple_coordinate({ apple_x, apple_y });
                            dfs_for_snake.set_snake_head_coordinate(this->snake_.get_head());
                        }
                        coord apple_coordinates = { apple_x, apple_y };
                        dfs_for_snake.DFS(apple_coordinates, { -1,-1 });
                            //any_neighbour(apple_y, apple_x, width, height));
                        dfs_for_snake.get_path(spath);
                        first_iteration = false;
                    }
                    auto& direction = spath.front();
                    snake_.move(direction);
                    spath.pop_front();
                }
                else if (_kbhit()) // проверяем, была ли нажата какая-либо клавиша
                {
                    int k = _getch(); // считываем код клавиши из буфера
                    if (k == 0 || k == 224) // если первый код - вспомогательный, считываем второй код
                        k = _getch();
                    switch (k) // пропускаем код нажатой клавиши внутрь оператора выбора
                    {
                    case 80: // вниз                        
                        if (snake_.get_direction() == direction::up)
                        {
                            continue;
                        }
                        snake_.move(direction::down);
                        break;
                    case 72: // вверх                        
                        if (snake_.get_direction() == direction::down)
                        {
                            continue;
                        }
                        snake_.move(direction::up);
                        break;
                    case 75: // влево                        
                        if (snake_.get_direction() == direction::right)
                        {
                            continue;
                        }
                        snake_.move(direction::left);
                        break;
                    case 77: // вправо                        
                        if (snake_.get_direction() == direction::left)
                        {
                            continue;
                        }
                        snake_.move(direction::right);
                        break;
                    case 27: // была нажата клавиша ESC
                        escape_key = true;
                        break;
                    }
                }
                else
                {
                    snake_.move();
                }

                if (snake_.is_collision())
                {
                    goto restart;
                }

                // отрисовка
                if (!escape_key)
                {
                    // смотрим, где сейчас находится голова
                    auto& head = snake_.get_head();

                    // был выход за рамки игрового поля?
                    if (!(head.x > 0 && head.x < width - 1 && head.y > 0 && head.y < height - 1))
                    {
                        lost = true;
                    }
                    else if (head.x == apple_x && head.y == apple_y) // съели яблоко?
                    {
                        clear({ apple_x, apple_y });
                        generate_apple_pos();                       
                        redraw_apple();
                                    
                        snake_.push_back(tail_before_move);
                        redraw_snake({ tail_before_move.x, tail_before_move.y }, { apple_x, apple_y });

                        inc_score();
                        print_score();
                    }
                    else
                    {
                        if (!snake_.is_collision())
                        {                            
                            clear({ tail_before_move.x, tail_before_move.y });
                            auto& coords = snake_.get_coordinates();
                            redraw_snake({ 0, 0 }, { coords.back().x, coords.back().y });
                        }
                    }
                }
                // когда проиграл или же вышел из игры
            } while (!lost && !escape_key);
            if (!escape_key)
            {
                goto restart;
            }
        }

        void clear(const COORD&& c)
        {
            SetConsoleCursorPosition(pingle::std_output_handle, c);
            putchar(' ');
        }

        void redraw_apple()
        {
            coord c = { apple_x, apple_y };
            draw(c, 12, apple_sym); // рисуем "яблоко" красного цвета
        }

        void redraw_snake(const coord& tail, const coord& head)
        {
            if (tail.x != 0) // рисовать хвост?
            {
                coord c = { tail.x, tail.y };
                draw(c, 12, snake_body_sym); // рисуем "хвост" зелёного цвета
            }
            if (head.x != 0) // рисовать голову?
            {
                coord c = { head.x, head.y };
                draw(c, 12, snake_head_sym);
            }
        }

        void draw(const coord& c, int color, char symbol)
        {
            COORD cd = { c.x, c.y };
            SetConsoleCursorPosition(pingle::std_output_handle, cd);
            SetConsoleTextAttribute(pingle::std_output_handle, color); // установка цвета
            putchar(symbol); // отображение символа
        }

    private:

        void inc_score()
        {
            ++score;
        }

        void print_score()
        {
            auto score_ = score;
            std::list<char> digits;
            SHORT number_width = 0;
            if (score_ == 0)
            {
                digits.push_front('0');
                ++number_width;
            }
            else
            {
                for (int i = score_ % 10; score_ != 0 ; score_ /= 10, i = score_ % 10)
                {
                    digits.push_front('0' + i);
                    ++number_width;
                }
            }
            for (SHORT x = width + 3 + number_width; x != width + 3; --x)
            {
                auto iterator = digits.cbegin();
                std::advance(iterator, x - (width + 3 + 1));
                draw({ x, 0 }, 4, *iterator);
            }
        }

        const int sleep_time;
        const unsigned int width; // размеры поля, по которому бегает змейка
        const unsigned int height; // размеры поля, по которому бегает змейка

        snake snake_;
        char snake_body_sym = '-'; // символ для отображения тела "змейки"
        char snake_head_sym = 1; // символ для отображения головы "змейки"
        char apple_sym = 'o'; // символ для отображения "яблока"

        SHORT apple_x; // абсцисса "яблока"
        SHORT apple_y; // ордината "яблока"

        menu menu_;
        bool is_demo;

        unsigned int score;
    };
}

int main()
{
    pingle::std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE); // создание хендла потока вывода
    CONSOLE_CURSOR_INFO cci = { sizeof(cci), false }; // создание параметров на отображение курсора
    SetConsoleCursorInfo(pingle::std_output_handle, &cci); //связывание параметров и хендла
    SetConsoleTextAttribute(pingle::std_output_handle, 4); // установка цвета, которым рисуется рамка поля
    pingle::window wnd;
    wnd.show();
}