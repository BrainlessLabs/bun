#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file DbLogger.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Creates and Returns a logger to log.
///////////////////////////////////////////////////////////////////////////////

#include "blib/utils/Singleton.hpp"
#include <spdlog/spdlog.h>

namespace blib {
  namespace bun {
    /////////////////////////////////////////////////
    /// @fn l
    /// @brief Returns the logger
    ///        This will be logging to the logs. It uses spdlog to log to file
    /// @return Returns the logger.
    /////////////////////////////////////////////////
    inline spdlog::logger& l() {
      static const std::size_t q_size = 1048576; //queue size must be power of 2
      //spdlog::set_async_mode( q_size );
      static auto ret = spdlog::daily_logger_st( "bun_logger",
                                                 "query_log.txt" );
	  static bool initialized = false;
	  if (!initialized) {
		  spdlog::set_pattern("***[%l] [%H:%M:%S %z] [thread %t] %v ***");
		  initialized = true;
	  }
	  
      return *ret;
    }
  }
}
