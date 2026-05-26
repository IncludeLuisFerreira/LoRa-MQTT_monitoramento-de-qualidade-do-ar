

#define TIME_BETWEEN_HIGH 250 //(ms)

/* @brief Funcao de utilidade para piscar led

  @param LED_PIN o numero do pino que esta o LED
  @param time_between o tempo entre o led piscar
  @param n_times quantas vezes o led pisca
*/
void blinkLED(int LED_PIN, long time_between, int n_times) {
  long lastReconnectAttempt = 0;
  int i = 0;
  
  while (i != n_times) {

    unsigned long now = millis();
    
    if (now - lastReconnectAttempt > time_between) {
      lastReconnectAttempt = now;
      digitalWrite(LED_PIN, HIGH);
      delay(TIME_BETWEEN_HIGH);
      digitalWrite(LED_PIN, LOW);
      i++;
    }
  }

}