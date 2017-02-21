# IoT--Kitchen_Cabinet_Lights_Public

##Demo - [Finished Product Before Mounting](https://www.youtube.com/watch?v=lRLasuZxU7c)

![Final Product](https://github.com/srfnmnk/IoT--Kitchen_Cabinet_Lights_Public/blob/master/images/WireUp.jpg)

###Quick Description

Basically, create a new lambda function in AWS and initialize it with the
Smart Home SDK/API. This will be your lambda function that controls Alexa integration.
This is the gist of the [cabLightsLambda.js](https://github.com/srfnmnk/IoT--Kitchen_Cabinet_Lights_Public/blob/master/cabLightsLambda.js)
code. Then you need to log into your Alexa app and authorize Alexa via OAuth v2 login screen to the particle account; a good [HowTo YouTube Video for joining Alexa to Particle](https://youtu.be/0rEBe_ZNBTk)
was created by a buddy from [Particle Community](https://community.particle.io). More details for creating Alexa's Lambda functions are linked below.

[Create Smart Home Skill for Alexa](https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/steps-to-create-a-smart-home-skill).
[Lambda Function for Alexa Skill Docs](https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/steps-to-create-a-smart-home-skill).
[SDK for Particle Photon Docs](https://docs.particle.io/guide/getting-started/intro/photon/).

Now, create a new product in the particle console and create a client auth key and use that in the code as that "Bearer" Auth token.

Finally, create your code to control your LEDs like you want; My code uses a photo diode to measure ambient room lighting and
change the light output via PWM based with some more function to control gitter and allow overrides to take place for an amount of time.

My example code for this is the [cabLightingFinal.ino](https://github.com/srfnmnk/IoT--Kitchen_Cabinet_Lights_Public/blob/master/CabLightingFinal.ino) but as
I said this can be much simpler.

Here's the wiring - ![WiringImg](https://github.com/srfnmnk/IoT--Kitchen_Cabinet_Lights_Public/blob/master/images/WireUp.jpg)

I hope to find some time to put together a more detailed readme on the Alexa setup and create a simple setup file but for now this will have to do.
