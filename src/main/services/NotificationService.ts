import { Toaster } from '@tser-framework/main';

import rogLogo from '@resources/icons/icon-512x512.png?asset';

class NotificationService {
  public toast(message: string): void {
    Toaster.toast(message, rogLogo);
  }
}

export const notificationService = new NotificationService();
