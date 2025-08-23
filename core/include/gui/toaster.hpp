#include <string>

#include "RccCommons.hpp"

class Toaster
{

public:
    static Toaster &getInstance()
    {
        static Toaster instance;
        return instance;
    }

    void showToast(std::string message, bool can_be_hidden = true, std::string icon = Constants::ICON_FILE);

private:
    uint last_id = 0;
};