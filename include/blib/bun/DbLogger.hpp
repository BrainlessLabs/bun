#pragma once
#include "blib/utils/Singleton.hpp"
#include <blib/spdlog/spdlog.h>

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
      spdlog::set_async_mode( q_size );
      static auto ret = spdlog::daily_logger_st( "async_file_logger",
        "query_log.txt" );
      return *ret;
    }
  }
}