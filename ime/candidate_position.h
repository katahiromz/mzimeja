#ifndef MZIMEJA_CANDIDATE_POSITION_H
#define MZIMEJA_CANDIDATE_POSITION_H

// Candidate and exclusion rectangles are in screen coordinates.
inline POINT CandidatePosExclude(POINT pt, const RECT& exclude, const RECT& work,
                                 SIZE size, BOOL vertical)
{
    if (vertical) {
        pt.x = exclude.left - size.cx;
        if (pt.x < work.left && exclude.right + size.cx <= work.right)
            pt.x = exclude.right;
    } else {
        pt.y = exclude.bottom;
        if (pt.y + size.cy > work.bottom && exclude.top - size.cy >= work.top)
            pt.y = exclude.top - size.cy;
    }

    if (pt.x + size.cx > work.right) pt.x = work.right - size.cx;
    if (pt.y + size.cy > work.bottom) pt.y = work.bottom - size.cy;
    if (pt.x < work.left) pt.x = work.left;
    if (pt.y < work.top) pt.y = work.top;
    return pt;
}

#endif
