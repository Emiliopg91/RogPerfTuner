import { Toaster } from '@tser-framework/main';

import rogLogo from '../../../resources/icons/icon-512x512.png?asset';

export class NotificationService {
  public static toast(message: string): void {
    Toaster.toast(message, rogLogo);
  }
}
