#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

struct MouseEvent {
    explicit MouseEvent(SDL_MouseButtonEvent mevent): event(mevent) { }
    SDL_MouseButtonEvent event;
};

#endif // EVENTS_H_INCLUDED
