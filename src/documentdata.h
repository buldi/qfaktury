#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef DOCUMENTDATA_H
#define DOCUMENTDATA_H

#include "fwd.hpp"

class DocumentData
{
public:
    virtual QString getCustomer() const = 0;

protected:
    ~DocumentData() = default;
};

#endif // DOCUMENTDATA_H
