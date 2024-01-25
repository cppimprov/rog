#pragma once

#include <memory>

namespace bump { class app; class gamestate; } // bump
namespace ta { struct world; namespace net { class server; } } // ta

namespace ta
{

	bump::gamestate loading(bump::app& app);
	bump::gamestate waiting_for_players(bump::app& app, std::unique_ptr<ta::world> world);
	bump::gamestate main_loop(bump::app& app, std::unique_ptr<ta::world> world, ta::net::server server);

} // ta
