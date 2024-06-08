/*
    Copyright 2021-2023 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <Picovoice_EN.h>

#include "Arduino_BMI270_BMM150.h"

unsigned long clocktime;

// include voice model
//#include "params.h"
#include "carDrive_en_cortexm_v3_0_0.h"

#include <Wire.h>

#define MEMORY_BUFFER_SIZE (70 * 1024)

static const char *ACCESS_KEY = "kOTLHLbXicVCqiZf/ulYSovGYup+rlyPYYCu72WuDbkIBv9qMQr/zg=="; // AccessKey string obtained from Picovoice Console (https://picovoice.ai/console/)

static pv_picovoice_t *handle = NULL;

static int8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));

static    byte x = 0;
static    byte y = 0;
static    byte y_prev = 0;

static    float accumuZyaw=0;
static    bool turn_trigger = false;

static const float PORCUPINE_SENSITIVITY = 0.75f;
static const float RHINO_SENSITIVITY = 0.5f;
static const float RHINO_ENDPOINT_DURATION_SEC = 1.0f;
static const bool RHINO_REQUIRE_ENDPOINT = true;

static void wake_word_callback(void) {
    //Serial.println("Wake word detected!");
}

static void inference_callback(pv_inference_t *inference) {
    //Serial.println("{");
    //Serial.print("    is_understood : ");
    //Serial.println(inference->is_understood ? "true" : "false");
    if (inference->is_understood) {
        //Serial.print("    intent : ");
        //Serial.println(inference->intent);
        if (inference->num_slots > 0) {
            //Serial.println("    slots : {");
            //Serial.print(inference->num_slots);
            if (*inference->values[0] == 's') y = 0; // stop
            else {
              if (*inference->values[0] == 'r') y = 1; // right
              if (*inference->values[0] == 'f') y = 2; // forward
              if (*inference->values[0] == 'b') y = 3; // backward
              if (*inference->values[0] == 'l') y = 4; // left
            }
            //Serial.print(y);

        Wire.beginTransmission(4); // transmit to device #4
        Wire.write("y is ");        // sends five bytes
        Wire.write(y);              // sends one byte
        Wire.endTransmission();    // stop transmitting

            for (int32_t i = 0; i < inference->num_slots; i++) {
                Serial.print("        ");
                Serial.print(inference->slots[i]);
                Serial.print(" : ");
                Serial.println(inference->values[i]);
            }
            Serial.println("    }");
        }
    }
    //Serial.println("}\n");
    pv_inference_delete(inference);
}

static void print_error_message(char **message_stack, int32_t message_stack_depth) {
    for (int32_t i = 0; i < message_stack_depth; i++) {
        //Serial.println(message_stack[i]);
    }
}

void setup() {

    Serial.begin(9600);
    
    Wire.begin(); // join i2c bus (address optional for master)

    if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
    }

    //Serial.begin(9600);
    //while (!Serial);

    pv_status_t status = pv_audio_rec_init();
    if (status != PV_STATUS_SUCCESS) {
        //Serial.print("Audio init failed with ");
        //Serial.println(pv_status_to_string(status));
        while (1);
    }

    char **message_stack = NULL;
    int32_t message_stack_depth = 0;
    pv_status_t error_status;



    status = pv_picovoice_init(
        ACCESS_KEY,
        MEMORY_BUFFER_SIZE,
        memory_buffer,
        sizeof(KEYWORD_ARRAY),
        KEYWORD_ARRAY,
        PORCUPINE_SENSITIVITY,
        wake_word_callback,
        sizeof(CONTEXT_ARRAY),
        CONTEXT_ARRAY,
        RHINO_SENSITIVITY,
        RHINO_ENDPOINT_DURATION_SEC,
        RHINO_REQUIRE_ENDPOINT,
        inference_callback,
        &handle);
    if (status != PV_STATUS_SUCCESS) {
        //Serial.print("Picovoice init failed with ");
        //Serial.println(pv_status_to_string(status));

        error_status = pv_get_error_stack(&message_stack, &message_stack_depth);
        if (error_status != PV_STATUS_SUCCESS) {
            //Serial.println("Unable to get Porcupine error state");
            while (1);
        }
        print_error_message(message_stack, message_stack_depth);
        pv_free_error_stack(message_stack);

        while (1);
    }

    const char *rhino_context = NULL;
    status = pv_picovoice_context_info(handle, &rhino_context);
    if (status != PV_STATUS_SUCCESS) {
        //Serial.print("retrieving context info failed with");
        //Serial.println(pv_status_to_string(status));
        while (1);
    }
    //Serial.println("Wake word: 'hey computer'");
    //Serial.println(rhino_context);
}

void loop() {
    const int16_t *buffer = pv_audio_rec_get_new_buffer();
    if (buffer) {
        const pv_status_t status = pv_picovoice_process(handle, buffer);
        if (status != PV_STATUS_SUCCESS) {
            //Serial.print("Picovoice process failed with ");
            //Serial.println(pv_status_to_string(status));
            char **message_stack = NULL;
            int32_t message_stack_depth = 0;
            pv_get_error_stack(
                &message_stack,
                &message_stack_depth);
            for (int32_t i = 0; i < message_stack_depth; i++) {
                //Serial.println(message_stack[i]);
            }
            pv_free_error_stack(message_stack);
            while (1);
        }

        //Wire.beginTransmission(4); // transmit to device #4
        //Wire.write("y is ");        // sends five bytes
        //Wire.write(y);              // sends one byte
        //Wire.endTransmission();    // stop transmitting

        //x++;
        //y = x % 2;

    }

    //if( y != 0) {
      //Serial.println(y);
    //}

  float xroll, ypitch, zyaw;
  clocktime = millis();

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(xroll, ypitch, zyaw);

    if (y != y_prev && (y == 1 || y == 4) ) {
      turn_trigger = true;
    }

    if (turn_trigger) {
      if (y == 4 || y == 1) {
        accumuZyaw = accumuZyaw + abs(zyaw) / (104.0/3); // 104 Hz sample rate
      }
      //clocktime;

      if (accumuZyaw >= 90) {
        y = 2; // going forward
        turn_trigger = false;
        accumuZyaw = 0;

        Wire.beginTransmission(4); // transmit to device #4
        Wire.write("y is ");        // sends five bytes
        Wire.write(y);              // sends one byte
        Wire.endTransmission();    // stop transmitting

      }

    }

    if( y != 0) {
      Serial.println(y);
      Serial.println(turn_trigger);
      Serial.println(accumuZyaw);
    }

  y_prev = y;

  }
}
