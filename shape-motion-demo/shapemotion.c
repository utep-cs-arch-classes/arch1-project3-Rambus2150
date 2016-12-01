/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>

#define GREEN_LED BIT6
static char i[1];
static int score;
static int score2;

static char j[1];
static int scorep2;
static int scorep2dig;

AbRect rect10 = {abRectGetBounds, abRectCheck, {2,10}}; /**< 10x10 rectangle */
AbRArrow rightArrow = {abRArrowGetBounds, abRArrowCheck, 15};

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 10, screenHeight/2 - 10}

};
/*
Layer layer4 = {
  (AbShape *)&rightArrow, 
  {(screenWidth/2)+20, (screenHeight/2)+20}, /**< bit below & right of center 
  {0,0}, {0,0},				    /* last & next pos 
  COLOR_PINK,
  0
};
  */

Layer layer3 = {		/**< Layer with violet circle */
  (AbShape *)&circle3,
  {(screenWidth/2), (screenHeight/2)}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_VIOLET,
  0
};


Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  0xa00,
  &layer3
};

Layer layer1 = {		/**< Layer with a red rect */
  (AbShape *)&rect10,
  {screenWidth-14, screenHeight/2}, /**< right side */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &fieldLayer,
};

Layer layer0 = {		/**< Layer with an orange rect */
  (AbShape *)&rect10,
  {15, (screenHeight/2)}, /**< left side */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_ORANGE,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
//MovLayer ml4 = { &layer4, {-1,-1}, 0 };
MovLayer ml3 = { &layer3, {1,1}, 0 };//ball /**< not all layers move */
MovLayer ml1 = { &layer1, {0,1}, &ml3}; //rectangle right
MovLayer ml0 = { &layer0, {0,-1}, &ml1}; //rectangle left





movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
	 
	Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */

	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated

}	  


//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
//
//bounce code
void mlBounce(MovLayer *ml, Region *paddleLeft, Region *paddleRight){
Vec2 newPos;
u_char axis;

Region shapeBoundary;
for(; ml;ml=ml->next){
vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);

// for (axis = 0; axis < 2; axis ++) {
     // 0 is col, 1 is row
//left paddle collision


	 if (((paddleLeft->topLeft.axes[1] <= shapeBoundary.botRight.axes[1]) &&
	    (shapeBoundary.topLeft.axes[0] <= paddleLeft->botRight.axes[0])&&
	    (paddleLeft->botRight.axes[1] >= shapeBoundary.topLeft.axes[1]))	
/*right paddle collision in if statement*/
	||((paddleRight->topLeft.axes[1] <= shapeBoundary.botRight.axes[1]) &&
	    (shapeBoundary.botRight.axes[0] >= paddleRight->topLeft.axes[0])&&
	    (paddleRight->botRight.axes[1] >= shapeBoundary.topLeft.axes[1]))){
	for (axis = 0; axis < 2; axis ++) {
     
int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	
	newPos.axes[axis] += (2*velocity);
      }
    }	/**< if outside of fence */
     /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}


//
void mlAdvance(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
//score but counts bounces here  	
	
//
	newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */
    } /**< for axis */
if(shapeBoundary.topLeft.axes[0]< fence->topLeft.axes[0]){
	scorep2++;
	if(scorep2>9){scorep2=0;
	scorep2dig++;
	if(scorep2dig>9)scorep2dig=0;
	}
	j[0]='0'+scorep2dig;
	j[1]= '0'+scorep2;
	}
if(shapeBoundary.botRight.axes[0]> fence->botRight.axes[0]){
   score++;
	if(score>9){score=0;
	score2++;
	if(score2>9)score2=0;
	}
	i[0]='0'+score2;
	i[1]= '0'+score;
}
	 ml->layer->posNext = newPos;
  } /**< for ml */
}


u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */
Region paddle0;//left
Region paddle1;//right
/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
 // switch_init();

  shapeInit();

  layerInit(&layer0);
  layerDraw(&layer0);
 

  
  
  
  layerGetBounds(&fieldLayer, &fieldFence);
  

  drawString5x7(5,0, "Welcome to pong", COLOR_GREEN, COLOR_BLACK);

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */


  for(;;) { 

    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);
	    /**< CPU OFF */

}
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
 
    movLayerDraw(&ml0, &layer0);
	
	drawString5x7(20,30, i, COLOR_GREEN, COLOR_BLACK);  
	drawString5x7(90,30, j ,COLOR_GREEN, COLOR_BLACK);  
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  
  if (count == 10) {

	layerGetBounds(&layer1, &paddle1);
  	layerGetBounds(&layer0, &paddle0);
    mlBounce(&ml3, &paddle0, &paddle1);
  
    mlAdvance(&ml0, &fieldFence);
    /*if (p2sw_read())
      redrawScreen = 1;*/
	redrawScreen = 1; 
   count = 0;
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
