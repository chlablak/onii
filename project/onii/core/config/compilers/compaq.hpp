#ifndef ONII_CORE_CONFIG_COMPILERS_COMPAQ_HPP
#define ONII_CORE_CONFIG_COMPILERS_COMPAQ_HPP

#if defined(__DECC) || defined(__DECCXX) || defined(__VAXC) || defined(VAXC)
# define ONII_COMPILER_COMPAQ 1
#endif

#endif // ONII_CORE_CONFIG_COMPILERS_COMPAQ_HPP